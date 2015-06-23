defmodule BTChip.HSM.Supervisor do
  use Supervisor

  def start_link() do
    :supervisor.start_link({:local, __MODULE__}, __MODULE__, [])
  end

  def init(_opts) do
    child = [
      supervisor(BTChip.HSM.Node.Supervisor, [], restart: :permanent),
      worker(BTChip.HSM.Node.Manager, [], restart: :permanent)
    ]
    supervise(child, strategy: :one_for_one)
  end

end
