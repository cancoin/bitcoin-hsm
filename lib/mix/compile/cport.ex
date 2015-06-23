defmodule Mix.Tasks.Compile.Cport do
  use Mix.Task

  @c_src Path.expand("../../../../c_src")

  def run(_args) do
    if Mix.shell.cmd("make -C " <> @c_src) != 0 do
      raise Mix.Error,
        message: "Failed to compile C-Port"
    end
  end
end
