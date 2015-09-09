defmodule Mix.Tasks.Compile.Cport do
  use Mix.Task

  @shortdoc "compile c-port in c_src"
  def run(_args) do
    if Mix.shell.cmd("make -C c_src") != 0 do
      raise Mix.Error,
        message: "Failed to compile C-Port"
    end
  end

  def clean do
    if Mix.shell.cmd("make -C c_src clean") != 0 do
      raise Mix.Error,
        message: "Failed to clean C-Port"
    end

  end
end


