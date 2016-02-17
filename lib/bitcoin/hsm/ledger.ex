defmodule Bitcoin.HSM.Ledger do
  use GenServer
  alias Bitcoin.HSM.Ledger.Manager

  @timeout 10000
  @port_opts [{:packet, 2}, :binary, :exit_status]

  defmodule State do
    defstruct [port: nil, location: nil]
  end

  def registered_name(%{address: address, bus: bus}) do
    :"#{__MODULE__}.Port#{address}.Bus#{bus}"
  end

  def start_link(location) do
    name = {:local, registered_name(location)}
    :gen_server.start_link(name, __MODULE__, [location], [{:timeout, @timeout}])
  end

  def init([location]) do
    Process.flag(:trap_exit, true)
    port = spawn_port(location)
    :ok = :gen_server.cast(Manager, {:register, self, location})
    {:ok, %State{location: location, port: port}}
  end

  def handle_call({:import, _type, _seed} = command, _from, %State{port: port} = state) do
    {:reply, call_command(command, port), state}
  end
  def handle_call({:derive, parent_key, index}, _from, %State{port: port} = state) do
    reply = call_command({:derive, parent_key, index}, port)
    {:reply, reply, state}
  end
  def handle_call({:pubkey, parent_key}, _from, %State{port: port} = state) do
    {:reply, call_command({:pubkey, parent_key}, port), state}
  end
  def handle_call({:sign, type, private_key, hash}, _from, %State{port: port} = state)
    when type in [:random, :deterministic] do
    {:reply, call_command({:sign, type, private_key, hash}, port), state}
  end
  def handle_call({:verify, _private_key, _hash, _signature} = command, _from, %State{port: port} = state) do
    {:reply, call_command(command, port), state}
  end
  def handle_call({:random, _count} = command, _from, %State{port: port} = state) do
    {:reply, call_command(command, port), state}
  end
  def handle_call({:pin, _pin} = command, _from, %State{port: port} = state) do
    {:reply, call_command(command, port), state}
  end

  def handle_info({port, {:data, binary}}, %State{port: port} = state) do
    handle_port(:erlang.binary_to_term(binary), state)
  end

  def terminate(reason, %State{port: port}) when is_port(port) do
    {:ok, :closed} = call_command({:close, reason}, port)
    :erlang.port_close(port)
    :ok
  end
  def terminate(_reason, _state) do
    :ok
  end

  defp handle_port({:error, :not_found}, state) do
    {:stop, :not_found, state}
  end

  defp spawn_port(location) do
    port_command = port_program ++ port_args(location)
    :erlang.open_port({:spawn, port_command}, @port_opts)
  end

  defp port_args(%{address: address, bus: bus}) do
    [' -p ', to_char_list(address), ' -b', to_char_list(bus)]
  end

  defp port_program do
    (:code.priv_dir(:bitcoin_hsm) ++ '/hsmport')
  end

  defp call_command(command, port) do
    command = :erlang.term_to_binary(command)
    true = :erlang.port_command(port, command)
    wait_response(port)
  end

  defp wait_response(port) do
    receive do
      {^port, {:data, :undef}} ->
        {:error, :undefined_function}
      {^port, {:data, response}} ->
        data = :erlang.binary_to_term(response)
        {:ok, data}
    after
      @timeout ->
        {:error, :timeout}
    end
  end

end
