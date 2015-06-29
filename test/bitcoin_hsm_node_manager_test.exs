defmodule BitcoinHsmManagerTest do
  use ExUnit.Case
  alias Bitcoin.HSM.Node.Manager

  test "list chips" do
    assert is_list(Manager.list_nodes!)
  end

end
