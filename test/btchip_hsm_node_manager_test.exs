defmodule BtchipHsmManagerTest do
  use ExUnit.Case
  alias BTChip.HSM.Node.Manager

  test "list chips" do
    assert is_list(Manager.list_nodes!)
  end

end
