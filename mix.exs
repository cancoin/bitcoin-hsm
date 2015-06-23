defmodule BtchipHsm.Mixfile do
  use Mix.Project

  def project do
    [app: :btchip_hsm,
     version: "0.0.1",
     elixir: "~> 1.0",
     build_embedded: Mix.env == :prod,
     start_permanent: Mix.env == :prod,
     compilers: Mix.compilers ++ [:cport, :asn1],
     asn1_paths: ["asn1"],
     erlc_paths: ["src"],
     asn1_options: [:ber, :der],
     deps: deps]
  end

  def application do
    [applications: [:sasl, :logger, :cure, :asn1ex],
     mod: {BTChip.HSM.App, []}]
  end

  defp deps do
    [
      {:cure, "~> 0.4.0"},
      {:excheck, "~> 0.2.3"},
      {:triq, git: "https://github.com/krestenkrab/triq.git"},
      {:asn1ex, git: "https://github.com/cancoin/asn1ex.git", branch: "wrap_source"}
    ]
  end

end
