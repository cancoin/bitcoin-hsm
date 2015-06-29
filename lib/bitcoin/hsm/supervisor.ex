defmodule Bitcoin.HSM.Supervisor do
  use Supervisor

  @timeout 10000

  def start_link() do
    :supervisor.start_link({:local, __MODULE__}, __MODULE__, [])
  end

  def init(_opts) do
    child = [
      supervisor(Bitcoin.HSM.Ledger.Supervisor, [], restart: :permanent, timeout: @timeout),
      worker(Bitcoin.HSM.Ledger.Manager, [], restart: :permanent)
    ]
    supervise(child, strategy: :one_for_one)
  end

end
