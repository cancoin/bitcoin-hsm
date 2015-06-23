defmodule Mix.Tasks.Clean.Cport do
  use Mix.Task
  import Mix.Generator

  def run(_) do
    {message, 0} = System.cmd("make", ["-C", "./c_src", "clean"], [])
    message |> String.split("\n") |> Enum.map &IO.inspect(&1)
  end
end
