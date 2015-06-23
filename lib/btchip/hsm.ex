defmodule BTChip.HSM do
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

  @process_group :btchip_hsm
  def process_group, do: @process_group

  @spec import_private_key(binary) :: {:ok, epk} | {:error, dongle_error}
  def import_private_key(seed) when is_binary(seed) do
    pick_hsm |> send_command({:import, seed})
  end

  @spec derive_bip32_key_path(epk, key_path) :: {:ok, epk} | {:error, dongle_error}
  def derive_bip32_key_path(parent_key, []), do: {:ok, parent_key}
  def derive_bip32_key_path(parent_key, [index|rest]) when is_integer(index) do
    case derive_bip32_key_path(parent_key, index) do
      {:ok, key} -> derive_bip32_key_path(key, rest)
      {:error, reason} -> {:error, reason}
    end
  end
  def derive_bip32_key_path(parent_key, <<"m", _ :: binary>> = path) do
    path_segments = parse_bip32_path(path)
    derive_bip32_key_path(parent_key, path_segments)
  end
  def derive_bip32_key_path(parent_key, index) when is_integer(index) do
    pick_hsm |> send_command({:derive, parent_key, index})
  end

  @spec get_public_key(epk) :: {:ok, extended_public_key} | {:error, dongle_error}
  def get_public_key(parent_key) do
    pick_hsm |> send_command({:public_key, parent_key})
  end

  @spec sign_immediate(epk, binary) :: {:ok, binary} | {:error, dongle_error}
  def sign_immediate(private_key, sighash) do
    pick_hsm |> send_command({:sign, private_key, sighash})
  end

  @spec verify_immediate(binary, sighash, binary) :: {:ok, true | false} | {:error, dongle_error}
  def verify_immediate(public_key, sighash, signature) do
    pick_hsm |> send_command({:verify, public_key, sighash})
  end

  @spec random(non_neg_integer) :: {:ok, binary} | {:error, dongle_error}
  def random(bytes) when is_integer(bytes) do
    pick_hsm |> send_command({:random, bytes})
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
    :gen_server.call(pid, command, @timeout)
  end

  defp pick_hsm do
    case :pg2.get_closest_pid(@process_group) do
      pid when is_pid(pid) -> pid
    end
  end

end
