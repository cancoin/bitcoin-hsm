defmodule BtchipHsmTest do
  use ExUnit.Case, async: false
  alias BTChip.HSM

  @vector1 %{
    seed: Base.decode16!("000102030405060708090A0B0C0D0E0F"),
    children: [
      %{path: "m", xpub: "xpub661MyMwAqRbcFtXgS5sYJABqqG9YLmC4Q1Rdap9gSE8NqtwybGhePY2gZ29ESFjqJoCu1Rupje8YtGqsefD265TMg7usUDFdp6W1EGMcet8"},
      %{path: "m/0p", xpub: "xpub68Gmy5EdvgibQVfPdqkBBCHxA5htiqg55crXYuXoQRKfDBFA1WEjWgP6LHhwBZeNK1VTsfTFUHCdrfp1bgwQ9xv5ski8PX9rL2dZXvgGDnw"},
      %{path: "m/0p/1", xpub: "xpub6ASuArnXKPbfEwhqN6e3mwBcDTgzisQN1wXN9BJcM47sSikHjJf3UFHKkNAWbWMiGj7Wf5uMash7SyYq527Hqck2AxYysAA7xmALppuCkwQ"},
      %{path: "m/0p/1/2p", xpub: "xpub6D4BDPcP2GT577Vvch3R8wDkScZWzQzMMUm3PWbmWvVJrZwQY4VUNgqFJPMM3No2dFDFGTsxxpG5uJh7n7epu4trkrX7x7DogT5Uv6fcLW5"},
      %{path: "m/0p/1/2p/2/1000000000p", xpub: "xpub6H1LXWLaKsWFhvm6RVpEL9P4KfRZSW7abD2ttkWP3SSQvnyA8FSVqNTEcYFgJS2UaFcxupHiYkro49S8yGasTvXEYBVPamhGW6cFJodrTHy"}
    ]
  }

  @vector2 %{
    seed: Base.decode16!("FFFCF9F6F3F0EDEAE7E4E1DEDBD8D5D2CFCCC9C6C3C0BDBAB7B4B1AEABA8A5A29F9C999693908D8A8784817E7B7875726F6C696663605D5A5754514E4B484542"),
    children: [
      %{path: "m", xpub: "xpub661MyMwAqRbcFW31YEwpkMuc5THy2PSt5bDMsktWQcFF8syAmRUapSCGu8ED9W6oDMSgv6Zz8idoc4a6mr8BDzTJY47LJhkJ8UB7WEGuduB"},
      %{path: "m/0", xpub: "xpub69H7F5d8KSRgmmdJg2KhpAK8SR3DjMwAdkxj3ZuxV27CprR9LgpeyGmXUbC6wb7ERfvrnKZjXoUmmDznezpbZb7ap6r1D3tgFxHmwMkQTPH"},
      %{path: "m/0/2147483647p", xpub: "xpub6ASAVgeehLbnwdqV6UKMHVzgqAG8Gr6riv3Fxxpj8ksbH9ebxaEyBLZ85ySDhKiLDBrQSARLq1uNRts8RuJiHjaDMBU4Zn9h8LZNnBC5y4a"},
      %{path: "m/0/2147483647p/1", xpub: "xpub6DF8uhdarytz3FWdA8TvFSvvAh8dP3283MY7p2V4SeE2wyWmG5mg5EwVvmdMVCQcoNJxGoWaU9DCWh89LojfZ537wTfunKau47EL2dhHKon"},
      %{path: "m/0/2147483647p/1/2147483647p", xpub: "xpub6ERApfZwUNrhLCkDtcHTcxd75RbzS1ed54G1LkBUHQVHQKqhMkhgbmJbZRkrgZw4koxb5JaHWkY4ALHY2grBGRjaDMzQLcgJvLJuZZvRcEL"},
      %{path: "m/0/2147483647p/1/2147483647p/2", xpub: "xpub6FnCn6nSzZAw5Tw7cgR9bi15UV96gLZhjDstkXXxvCLsUXBGXPdSnLFbdpq8p9HmGsApME5hQTZ3emM2rnY5agb9rXpVGyy3bdW6EEgAtqt"}
    ]
  }

  @vectors [@vector1, @vector2]

  @wif "5Kb8kLf9zgWQnogidDA76MzPL6TsZZY36hWXMssSzNydYXYB9KF"

  test "parse bip32 path" do
    assert [0, 0x80000000, 0x80000001, 0xFFFFFFFF] = HSM.parse_bip32_path("m/0/0p/1'/2147483647p")
    for %{children: children} <- @vectors do
      for %{path: path} <- children do
        for segment <- HSM.parse_bip32_path(path) do
          assert is_integer(segment)
        end
      end
    end
  end

  test "random" do
    {:ok, random} = HSM.random(8)
    assert String.length(random) <= 8
  end

  test "import bip32 seed" do
    for %{seed: seed} <- @vectors do
      {:ok, encoded_seed} = HSM.import_bip32_seed(seed)
      IO.inspect encoded_seed
    end
  end

  test "import private key" do
    {:ok, encoded_seed} = HSM.import_private_key(@wif)
  end

end
