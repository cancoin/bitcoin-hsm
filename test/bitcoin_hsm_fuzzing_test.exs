defmodule BitcoinHsmFuzzingTest do
  use ExUnit.Case, async: false
  use ExCheck
  alias Bitcoin.HSM

  @tag timeout: 600_000

  @hmin 0x80000000
  @hmax 0x100000000

  property :import_seed do
    for_all seed in binary(32) do
      {:ok, <<1, 2, 0, _rest :: binary>> = epk} = HSM.import_seed(seed)
      true
    end
  end

  property :private_derive do
    seed_bin = :crypto.strong_rand_bytes(32)
    {:ok, <<1, 2, 0, _rest :: binary>> = epk} = HSM.import_seed(seed_bin)
    for_all index in int(@hmin, @hmax) do
      {:ok, <<1, 2, 0, _rest :: binary>> = epk} = HSM.derive(epk, index)
      index >= @hmin
    end
  end

  property :public_derive do
    seed_bin = :crypto.strong_rand_bytes(32)
    {:ok, <<1, 2, 0, _rest :: binary>> = epk} = HSM.import_seed(seed_bin)
    for_all index in int(0, @hmin - 1) do
      {:ok, <<1, 2, 0, _rest :: binary>> = epk} = HSM.derive(epk, index)
      true
    end
  end

  property :derive_path do
    seed_bin = :crypto.strong_rand_bytes(32)
    {:ok, seed} = HSM.import_seed(seed_bin)
    for_all {a,b,c} in {int(0, @hmax), int(0, @hmax), int(0, @hmax)} do
      segments = Enum.map [a,b,c], fn
        (num) when num >= @hmin -> "#{num - @hmin}h"
        (num) -> to_string(num)
      end
      path = Enum.join(["m"] ++ segments, "/")
      {:ok, <<1, 2, 0, _rest :: binary>> = epk} = HSM.derive(seed, path)
      true
    end
  end

  property :public_key do
    for_all seed in binary(32) do
      {:ok, <<1, 2, 0, _rest :: binary>> = epk} = HSM.import_seed(seed)
      {:ok, %{public_key: public_key}} = HSM.public_key(epk)
      true
    end
  end

  property :sign_immediate do
    for_all {seed, sighash} in {binary(32), binary(32)} do
      {:ok, epk} = HSM.import_seed(seed)
      {:ok,  <<48, _ :: binary>> = signature} = HSM.sign(epk, sighash)
      IO.inspect {:epk, Base.encode16 signature}
      true
    end
  end

  property :verify_immediate do
    for_all {seed, sighash} in {binary(32), binary(32)} do
     {:ok, epk} = HSM.import_seed(seed)
      {:ok, <<48, _ :: binary>> = signature} = HSM.sign(epk, sighash)
      {:ok, %{public_key: public_key}} = HSM.public_key(epk)
      {:ok, true} = HSM.verify(public_key, sighash, signature)
      true
    end
  end

end
