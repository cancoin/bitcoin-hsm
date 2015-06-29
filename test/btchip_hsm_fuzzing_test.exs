if Mix.env == :fuzz do

  defmodule BtchipHsmTest do
    use ExUnit.Case
    use ExCheck
    alias BTChip.HSM

    @hmin 0x80000000
    @hmax 0x100000000

    property :import_private_key do
      for_all seed in binary(32) do
        {:ok, epk} = HSM.import_private_key(seed)
        true
      end
    end

    property :private_derive do
      seed_bin = :crypto.strong_rand_bytes(32)
      {:ok, epk} = HSM.import_private_key(seed_bin)
      for_all index in int(@hmin, @hmax) do
        {:ok, epk} = HSM.derive_bip32_key_path(epk, index)
        index >= @hmin
      end
    end

    property :public_derive do
      seed_bin = :crypto.strong_rand_bytes(32)
      {:ok, epk} = HSM.import_private_key(seed_bin)
      for_all index in int(0, @hmin - 1) do
        {:ok, epk} = HSM.derive_bip32_key_path(epk, index)
        true
      end
    end

    property :derive_path do
      seed_bin = :crypto.strong_rand_bytes(32)
      {:ok, seed} = HSM.import_private_key(seed_bin)
      for_all {a,b,c} in {int(0, @hmax), int(0, @hmax), int(0, @hmax)} do
        segments = Enum.map [a,b,c], fn
          (num) when num >= @hmin -> "'#{num - @hmin}"
          (num) -> to_string(num)
        end
        path = Enum.join(["m"] ++ segments, "/")
        {:ok, epk} = HSM.derive_bip32_key_path(seed, path)
        true
      end
    end

    property :get_public_key do
      for_all seed in binary(32) do
        {:ok, epk} = HSM.import_private_key(seed)
        {:ok, %{public_key: public_key}} = HSM.get_public_key(epk)
        true
      end
    end

    property :sign_immediate do
      for_all {seed, sighash} in {binary(32), binary(32)} do
        {:ok, seed} = HSM.import_private_key(seed)
        {:ok, signature} = HSM.sign_immediate(seed, sighash)
        true
      end
    end

    property :verify_immediate do
      for_all {seed, sighash} in {binary(32), binary(32)} do
        {:ok, signature} = HSM.sign_immediate(seed, sighash)
        {:ok, %{public_key: public_key}} = HSM.get_public_key(seed)
        {:ok, true} = HSM.verify_immediate(public_key, sighash, signature)
        true
      end
    end

  end

end
