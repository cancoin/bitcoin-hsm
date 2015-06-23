defmodule BTChip.HSM.Node do
  use GenServer
  alias BTChip.HSM.Node.Manager

  @timeout 10000
  @port_opts [{:packet, 2}, :binary, :exit_status]

  defmodule State do
    defstruct [port: nil, location: nil]
  end

  def registered_name(%{port: port, bus: bus}) do
    :"#{__MODULE__}.Port#{port}.Bus#{bus}"
  end

  def start_link(location) do
    name = {:local, registered_name(location)}
    :gen_server.start_link(name, __MODULE__, [location], [])
  end

  def init([location]) do
    port = spawn_port(location)
    :ok = :gen_server.cast(Manager, {:register, self, location})
    {:ok, %State{location: location, port: port}}
  end

  def handle_call({:derive, parent_key, index} = command, _from, state) do
    reply = call_command(command, state)
    {:reply, reply, state}
  end

  def handle_info({port, {:data, binary}}, %State{port: port} = state) do
    handle_port(:erlang.binary_to_term(binary), state)
  end

  defp handle_port({:error, :not_found}, state) do
    {:stop, :not_found, state}
  end

  defp spawn_port(location) do
    port_command = port_program ++ port_args(location)
    :erlang.open_port({:spawn, port_command}, @port_opts)
  end

  defp port_args(%{port: port, bus: bus}) do
    [' -p ', to_char_list(port), ' -b', to_char_list(bus)]
  end

  defp port_program do
    (:code.priv_dir(:btchip_hsm) ++ '/hsmport')
  end

  defp call_command(command, %State{port: port} = state) do
    command = :erlang.term_to_binary(command)
    true = :erlang.port_command(port, command)
    wait_response(state)
  end

  defp wait_response(%State{port: port} = state) do
    receive do
      {^port, {:data, :undef}} ->
        {:error, :undefined_function, state}
      {^port, {:data, response}} ->
        data = :erlang.binary_to_term(response)
        IO.inspect {:PORTDATA, data}
        {:ok, data, state}
    after
      @timeout ->
        {:error, :timeout, state}
    end
  end

end
