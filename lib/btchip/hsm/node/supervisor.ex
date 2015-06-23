defmodule BTChip.HSM.Node.Supervisor do
  use Supervisor

  def start_link() do
    :supervisor.start_link({:local, __MODULE__}, __MODULE__, [])
  end

  def start_child(%{port: port, bus: bus} = location) do
    :supervisor.start_child(__MODULE__, [location])
  end

  def terminate_child(child) do
    :supervisor.terminate_child(__MODULE__, child)
  end

  def init(_opts) do
    child = [
      worker(BTChip.HSM.Node, [], restart: :permanent)
    ]
    supervise(child, strategy: :simple_one_for_one)
  end

end
