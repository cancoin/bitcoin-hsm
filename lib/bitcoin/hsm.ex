defmodule Bitcoin.HSM do
  alias Bitcoin.HSM.Ledger.Manager
  require Integer

  @type epk :: binary
  @type public_index :: 0..0x7FFFFFFFFF
  @type private_index :: 0x80000000..0x100000000
  @type index :: 0..0x100000000
  @type key_path :: index | [index|list] | binary
  @type extended_public_key :: %{
    public_key: binary, chain_code: binary, depth: non_neg_integer,
    fingerprint: binary, child_number: integer}
  @type sighash :: binary
  @type dongle_error :: :dongle_not_found | :dongle_error

  @timeout :infinity
  @hardened_min 0x80000000
  @hardened_max 0x100000000
  @hardened_regex ~r/^(?<hardened>\d*)(p|P|h|H|')$/

  @mainnet_priv 0x0488ADE4
  @mainnet_pub 0x0488B21E
  @testnet_priv 0x043587CF
  @testnet_pub 0x04358394
  @version_pub @mainnet_pub
  @version_priv @mainnet_priv

  @process_group :bitcoin_hsm

  @asn1_mod :'BTChip-HSM-ECDSA'
  @asn1_seq :'ECDSA-Sig-Value-Seq'
  @asn1_seq_prefix 48
  @asn1_set :'ECDSA-Sig-Value-Set'
  @asn1_set_prefix 49


  def process_group, do: @process_group

  @spec import_wif(binary) :: {:ok, epk} | {:error, dongle_error}
  def import_wif(seed) when is_binary(seed) do
    pick_hsm |> send_command({:import, :wif, seed})
  end

  @spec import_seed(binary) :: {:ok, epk} | {:error, dongle_error}
  def import_seed(seed) when is_binary(seed) do
    pick_hsm |> send_command({:import, :seed, seed})
  end

  @spec derive(epk, key_path) :: {:ok, epk} | {:error, dongle_error}
  def derive(parent_key, []), do: {:ok, parent_key}
  def derive(parent_key, [index|rest]) when is_integer(index) do
    case derive(parent_key, index) do
      {:ok, key} -> derive(key, rest)
      {:error, reason} -> {:error, reason}
    end
  end
  def derive(parent_key, <<"m", _ :: binary>> = path) do
    path_segments = parse_bip32_path(path)
    derive(parent_key, path_segments)
  end
  def derive(parent_key, index) when is_integer(index) do
    pick_hsm |> send_command({:derive, parent_key, index})
  end

  @spec public_key(epk) :: {:ok, extended_public_key | binary} | {:error, dongle_error}
  def public_key(parent_key) do
    case pick_hsm |> send_command({:pubkey, parent_key}) do
      {:ok, reply} when is_list(reply) ->
        {:ok, Enum.into(reply, %{})}
      error ->
        error
    end
  end

  @spec serialize_public_key(extended_public_key) :: {:ok, binary} | {:error, atom}
  def serialize_public_key(xpub) when is_map(xpub) do
    public_key = case xpub[:public_key] do
      <<0x04, x::unsigned-size(256), y::unsigned-size(256)>> ->
       if Integer.is_odd(y) do
         <<0x03>> <> <<x::unsigned-size(256)>>
       else
         <<0x02>> <> <<x::unsigned-size(256)>>
       end
    end
    serialized = << @version_pub :: unsigned-size(32), xpub[:depth] :: unsigned-integer-size(8),
       xpub[:fingerprint] :: binary-size(4), xpub[:child_number] :: binary-size(4),
       xpub[:chain_code] :: binary-size(32), public_key :: binary-size(33)>>

    Base58Check.encode58check("", serialized)
  end

  @spec sign(epk, binary, :deterministic | :random) :: {:ok, binary} | {:error, dongle_error}
  def sign(private_key, sighash, type \\ :deterministic) do
    case pick_hsm |> send_command({:sign, type, private_key, sighash}) do
      {:ok, signature} ->
        transcode_signature(signature)
      error ->
        error
    end
  end

  @spec verify(binary, sighash, binary) :: {:ok, true | false} | {:error, dongle_error}
  def verify(public_key, sighash, signature) do
    pick_hsm |> send_command({:verify, public_key, sighash, signature})
  end

  @spec random(non_neg_integer) :: {:ok, binary} | {:error, dongle_error}
  def random(bytes) when is_integer(bytes) do
    pick_hsm |> send_command({:random, bytes})
  end

  @spec pin(binary) :: {:ok, :verified} | {:error, dongle_error}
  def pin(pin) do
    Manager.verify_pin!(pin)
  end

  def parse_bip32_path("m"), do: []
  def parse_bip32_path(<<"m/", path :: binary>>) do
    path
      |> String.split("/")
      |> Enum.reduce([], fn(segment, acc) ->
        case Regex.named_captures(@hardened_regex, segment) do
          %{"hardened" => index} ->
            [@hardened_min + String.to_integer(index)|acc]
          nil ->
            [String.to_integer(segment)|acc]
        end
      end)
      |> Enum.reverse
  end

  def transcode_signature(<<@asn1_set_prefix, _ :: binary>> = signature) do
    case @asn1_mod.decode(@asn1_set, signature) do
      {:ok, [_r, _v] = decoded_signature} ->
        @asn1_mod.encode(@asn1_seq, decoded_signature)
      error ->
        error
    end
  end
  def transcode_signature(<<@asn1_seq_prefix, _ :: binary>> = signature) do
    case @asn1_mod.decode(@asn1_seq, signature) do
      {:ok, [_r, _v] = decoded_signature} ->
        @asn1_mod.encode(@asn1_seq, decoded_signature)
      error ->
        error
    end
  end

  defp send_command({:no_process, @process_group}, _command) do
    {:error, :dongle_not_found}
  end
  defp send_command(pid, command) when is_pid(pid) do
    case :gen_server.call(pid, command, @timeout) do
      {:ok, {:error, _reason} = error} -> error
      {:ok, reply} -> reply
      error -> error
    end
  end

  defp pick_hsm do
    case :pg2.get_closest_pid(@process_group) do
      pid when is_pid(pid) -> pid
    end
  end

end
