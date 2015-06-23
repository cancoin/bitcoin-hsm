defmodule BtchipHsmManagerTest do
  use ExUnit.Case
  alias BTChip.HSM.Chip.Manager

  test "list chips" do
    assert is_list(Manager.list_chips)
  end

end
