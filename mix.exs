defmodule BitcoinHSM.Mixfile do
  use Mix.Project

  def project do
    [app: :bitcoin_hsm,
     version: "0.0.1",
     elixir: "~> 1.0",
     build_embedded: Mix.env == :prod,
     start_permanent: Mix.env == :prod,
     compilers: Mix.compilers ++ [:cport],
     asn1_paths: ["asn1"],
     erlc_paths: ["src"],
     asn1_options: [:ber, :der],
     deps: deps(Mix.env)]
  end

  def application do
    [applications: [:sasl, :logger],
     mod: {Bitcoin.HSM.App, []}]
  end

  defp deps(:doc) do
    [
      {:earmark, "~> 0.1", only: :dev},
      {:ex_doc, "~> 0.7", only: :dev}
    ] ++ deps(:prod)
  end

  defp deps(:test) do
    [
      {:triq, git: "https://github.com/krestenkrab/triq.git"}
    ] ++ deps(:prod)
  end

  defp deps(_env) do
    [
      {:base58check, github: "gjaldon/base58check"}
    ]
  end

end
