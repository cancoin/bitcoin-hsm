defmodule Mix.Tasks.Btchip.Hsm.WriteNodeList do
  use Mix.Task
  import Mix.Generator

  def run(_) do
    {:ok, nodes} = BTChip.HSM.Node.Manager.list_nodes
    IO.inspect nodes
    :ok = File.write('./priv/nodes.config', :io_lib.format("~p.~n", nodes))
  end
end
