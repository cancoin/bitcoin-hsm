defmodule BTChip.HSM.App do

  use Application

  def start(_, _) do
    BTChip.HSM.Supervisor.start_link
  end

end
