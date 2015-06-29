defmodule Bitcoin.HSM.App do

  use Application

  def start(_, _) do
    Bitcoin.HSM.Supervisor.start_link
  end

end
