defmodule BitcoinHsmTest do
  use ExUnit.Case, async: false
  alias Bitcoin.HSM

  @tag timeout: 60_000

  @vector1 %{
    seed: Base.decode16!("000102030405060708090A0B0C0D0E0F"),
    children: [
      %{path: "m", xpub: "xpub661MyMwAqRbcFtXgS5sYJABqqG9YLmC4Q1Rdap9gSE8NqtwybGhePY2gZ29ESFjqJoCu1Rupje8YtGqsefD265TMg7usUDFdp6W1EGMcet8"},
      %{path: "m/0p", xpub: "xpub68Gmy5EdvgibQVfPdqkBBCHxA5htiqg55crXYuXoQRKfDBFA1WEjWgP6LHhwBZeNK1VTsfTFUHCdrfp1bgwQ9xv5ski8PX9rL2dZXvgGDnw"},
      %{path: "m/0p/1", xpub: "xpub6ASuArnXKPbfEwhqN6e3mwBcDTgzisQN1wXN9BJcM47sSikHjJf3UFHKkNAWbWMiGj7Wf5uMash7SyYq527Hqck2AxYysAA7xmALppuCkwQ"},
      %{path: "m/0p/1/2p", xpub: "xpub6D4BDPcP2GT577Vvch3R8wDkScZWzQzMMUm3PWbmWvVJrZwQY4VUNgqFJPMM3No2dFDFGTsxxpG5uJh7n7epu4trkrX7x7DogT5Uv6fcLW5"},
      %{path: "m/0p/1/2p/2/1000000000", xpub: "xpub6H1LXWLaKsWFhvm6RVpEL9P4KfRZSW7abD2ttkWP3SSQvnyA8FSVqNTEcYFgJS2UaFcxupHiYkro49S8yGasTvXEYBVPamhGW6cFJodrTHy"}
    ]
  }

  @vector2 %{
    seed: Base.decode16!("FFFCF9F6F3F0EDEAE7E4E1DEDBD8D5D2CFCCC9C6C3C0BDBAB7B4B1AEABA8A5A29F9C999693908D8A8784817E7B7875726F6C696663605D5A5754514E4B484542"),
    children: [
      %{path: "m", xpub: "xpub661MyMwAqRbcFW31YEwpkMuc5THy2PSt5bDMsktWQcFF8syAmRUapSCGu8ED9W6oDMSgv6Zz8idoc4a6mr8BDzTJY47LJhkJ8UB7WEGuduB"},
      %{path: "m/0", xpub: "xpub69H7F5d8KSRgmmdJg2KhpAK8SR3DjMwAdkxj3ZuxV27CprR9LgpeyGmXUbC6wb7ERfvrnKZjXoUmmDznezpbZb7ap6r1D3tgFxHmwMkQTPH"},
      %{path: "m/0/2147483647p", xpub: "xpub6ASAVgeehLbnwdqV6UKMHVzgqAG8Gr6riv3Fxxpj8ksbH9ebxaEyBLZ85ySDhKiLDBrQSARLq1uNRts8RuJiHjaDMBU4Zn9h8LZNnBC5y4a"},
      %{path: "m/0/2147483647p/1", xpub: "xpub6DF8uhdarytz3FWdA8TvFSvvAh8dP3283MY7p2V4SeE2wyWmG5mg5EwVvmdMVCQcoNJxGoWaU9DCWh89LojfZ537wTfunKau47EL2dhHKon"},
      %{path: "m/0/2147483647p/1/2147483646p", xpub: "xpub6ERApfZwUNrhLCkDtcHTcxd75RbzS1ed54G1LkBUHQVHQKqhMkhgbmJbZRkrgZw4koxb5JaHWkY4ALHY2grBGRjaDMzQLcgJvLJuZZvRcEL"},
      %{path: "m/0/2147483647p/1/2147483646p/2", xpub: "xpub6FnCn6nSzZAw5Tw7cgR9bi15UV96gLZhjDstkXXxvCLsUXBGXPdSnLFbdpq8p9HmGsApME5hQTZ3emM2rnY5agb9rXpVGyy3bdW6EEgAtqt"}
    ]
  }

  @vectors [@vector1, @vector2]

  @wif "5Kb8kLf9zgWQnogidDA76MzPL6TsZZY36hWXMssSzNydYXYB9KF"
  @hash Base.decode16!("B7405DAEBB63964BA8EBD35933A56538FAFD131D9FDA4FC463D926F6F736CF57")

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
      {:ok, epk} = HSM.import_seed(seed)
      assert <<1, 2, 0, _ :: binary>> = epk
    end
  end

  test "import private key" do
    {:ok, epk} = HSM.import_wif(@wif)
    assert <<1, 1, 0, _ :: binary>> = epk
  end

  test "get public key bip32" do
    for %{seed: seed, children: children} <- @vectors do
      {:ok, epk} = HSM.import_seed(seed)
      {:ok, %{public_key: pubkey} = xpub} = HSM.public_key(epk)
      [serialized_key] = for %{path: "m", xpub: serialized_key} <- children, do: serialized_key
      assert serialized_key == HSM.serialize_public_key(xpub)
    end
  end

  test "get public key base58" do
    {:ok, epk} = HSM.import_wif(@wif)
    {:ok, %{public_key: pubkey}} = HSM.public_key(epk)
    assert String.length(pubkey) == 64
  end

  test "derive" do
    for %{seed: seed, children: children} <- @vectors do
      {:ok, master_epk} = HSM.import_seed(seed)
      for %{path: path, xpub: xpub} <- children do
        {:ok, epk} = HSM.derive(master_epk, path)
        {:ok, public_key} = HSM.public_key(epk)
        assert xpub == HSM.serialize_public_key(public_key)
      end
    end
  end

  test "sign" do
    for %{seed: seed} <- @vectors do
      {:ok, epk} = HSM.import_seed(seed)
      {:ok, signature} = HSM.sign(epk, @hash)
      assert <<48, _ :: binary>> = signature
    end
  end

  test "verify" do
    for %{seed: seed} <- @vectors do
      {:ok, epk} = HSM.import_seed(seed)
      {:ok, signature} = HSM.sign(epk, @hash)
      {:ok, %{public_key: pubkey}} = HSM.public_key(epk)
      assert {:ok, true} == HSM.verify(pubkey, @hash, signature)
    end
  end

end
