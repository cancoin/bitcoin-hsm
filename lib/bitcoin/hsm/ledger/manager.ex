defmodule Bitcoin.HSM.Ledger.Manager do
  use GenServer

  import Bitcoin.HSM, only: [process_group: 0]

  alias Bitcoin.HSM.Ledger

  @port_opts [{:packet, 2}, :binary]
  @timeout 10000

  defmodule State do
    defstruct [nodes: []]
  end

  def start_nodes do
    nodes = Enum.reduce list_nodes, %{}, fn(location, acc) ->
      {:ok, ref, location} = start_node(location)
      Map.put(acc, ref, location)
    end
    {:ok, nodes}
  end

  def list_nodes do
    {:ok, nodes} = list_nodes_port
    Enum.map(nodes, &Enum.into(&1, %{}))
  end

  def start_node(location) do
    node_pid = ensure_started(location)
    ref = Process.monitor(node_pid)
    location = Map.merge(location, %{pid: node_pid})
    {:ok, ref, location}
  end

  def verify_pin!(pin) do
    Enum.each list_nodes, fn(node) ->
      {:ok, :verified} = node
        |> Ledger.registered_name
        |> Process.whereis
        |> :gen_server.call({:pin, pin})
    end
  end

  def verify_pin!(pin, []) do
    {:ok, :verified}
  end
  def verify_pin!(pin, [node|nodes]) do
    reply = node
      |> Ledger.registered_name
      |> Process.whereis
      |> :gen_server.call({:pin, pin})
    case reply do
      {:ok, :verified} ->
        verify_pin!(pin, nodes)
      error -> error
    end
  end


  def start_link do
    :gen_server.start_link({:local, __MODULE__}, __MODULE__, [], [{:timeout, @timeout}])
  end

  def init(_) do
    create_process_group
    {:ok, nodes} = start_nodes
    {:ok, %State{nodes: nodes}}
  end

  def terminate(_reason, %State{nodes: nodes}) do
    for %{pid: node} <- nodes do
      {:ok, :closed} = :gen_server.call(node, :close)
    end
    :ok
  end

  def handle_cast({:register, pid, _location}, state) do
    :ok = :pg2.join(process_group, pid)
    {:noreply, state}
  end

  def handle_info({:'DOWN', monitor_ref, :process, _pod, _reason}, %State{nodes: nodes} = state) do
    {:noreply, %State{state | nodes: Map.delete(nodes, monitor_ref)}}
  end

  def create_process_group do
    :pg2.create(process_group)
  end

  defp ensure_started(location) do
    case Bitcoin.HSM.Ledger.Supervisor.start_child(location) do
      {:ok, pid} -> pid
      {:error, {:already_started, pid}} -> pid
    end
  end

  defp list_nodes_port do
    port = :erlang.open_port({:spawn, list_nodes_program}, @port_opts)
    receive_node_list(port)
  end

  defp receive_node_list(port) do
    receive do
      {^port, {:data, binary}} ->
        nodes = :erlang.binary_to_term(binary)
        {:ok, nodes}
      {^port, other} ->
        receive_node_list(port)
        IO.inspect(other)
    after
      5000 -> {:error, :timeout}
    end
  end

  defp list_nodes_program do
    (:code.priv_dir(:bitcoin_hsm) ++ '/hsmlist') |> to_string
  end

  defp list_nodes_file do
    (:code.priv_dir(:bitcoin_hsm) ++ '/nodes.config') |> to_string
  end


end
