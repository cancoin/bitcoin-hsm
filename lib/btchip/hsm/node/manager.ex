defmodule BTChip.HSM.Node.Manager do
  use GenServer

  import BTChip.HSM, only: [process_group: 0]

  @port_opts [{:packet, 2}, :binary]
  @timeout 10000

  defmodule State do
    defstruct [nodes: []]
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

  def create_process_group do
    :pg2.create(process_group)
  end

  def start_nodes do
    {:ok, nodes} = list_nodes
    nodes = Enum.reduce nodes, %{}, fn(location, acc) ->
      location = Enum.into(location, %{})
      node_pid = ensure_started(location)
      mon_ref = Process.monitor(node_pid)
      location = Map.merge(location, %{pid: node_pid})
      Map.put(acc, mon_ref, location)
    end
    {:ok, nodes}
  end

  def list_nodes do
    case :file.consult(list_nodes_file) do
      {:ok, nodes} when is_list(nodes) -> {:ok, nodes}
      {:error, :enoent} -> list_nodes_port
    end
  end

  def list_nodes_port do
    port = :erlang.open_port({:spawn, list_nodes_program}, @port_opts)
    receive_node_list(port)
  end

  def handle_cast({:register, pid, _location}, state) do
    :ok = :pg2.join(process_group, pid)
    {:noreply, state}
  end

  def handle_info({:'DOWN', mon_ref, :process, _pod, _reason}, %State{nodes: nodes} = state) do
    {:noreply, %State{state | nodes: Map.delete(nodes, mon_ref)}}
  end

  defp ensure_started(location) do
    case BTChip.HSM.Node.Supervisor.start_child(location) do
      {:ok, pid} -> pid
      {:error, {:already_started, pid}} -> pid
    end
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
    (:code.priv_dir(:btchip_hsm) ++ '/hsmlist') |> to_string
  end

  defp list_nodes_file do
    (:code.priv_dir(:btchip_hsm) ++ '/nodes.config') |> to_string
  end


end
