defmodule Bitcoin.HSM do
  alias Bitcoin.HSM.Ledger.Manager

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

  @process_group :bitcoin_hsm
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

  @spec public_key(epk) :: {:ok, extended_public_key} | {:error, dongle_error}
  def public_key(parent_key) do
    case pick_hsm |> send_command({:pubkey, parent_key}) do
      {:ok, reply} when is_list(reply) -> {:ok, Enum.into(reply, %{})}
      error -> error
    end
  end

  @spec sign(epk, binary, :deterministic | :random) :: {:ok, binary} | {:error, dongle_error}
  def sign(private_key, sighash, type \\ :deterministic) do
    pick_hsm |> send_command({:sign, type, private_key, sighash})
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
    segments = path
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

  defp send_command({:no_process, @process_group} = error, _command) do
    {:error, :dongle_not_found}
  end
  defp send_command(pid, command) when is_pid(pid) do
    case :gen_server.call(pid, command, @timeout) do
      {:ok, reply} -> reply
      {:ok, {:error, _reason} = error} -> error
      error -> error
    end
  end

  defp pick_hsm do
    case :pg2.get_closest_pid(@process_group) do
      pid when is_pid(pid) -> pid
    end
  end

end
