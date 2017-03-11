// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Weapons/MasterWeapon.h"
#include "FusionWeapon_Projectile.generated.h"

USTRUCT()
struct FProjectileWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** projectile class */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AFusionProjectile> ProjectileClass;

	/** life time */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float ProjectileLife;

	/** damage at impact point */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	int32 ExplosionDamage;

	/** radius of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float ExplosionRadius;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	TSubclassOf<UDamageType> DamageType;

	/** defaults */
	FProjectileWeaponData()
	{
		ProjectileClass = NULL;
		ProjectileLife = 10.0f;
		ExplosionDamage = 100;
		ExplosionRadius = 300.0f;
		DamageType = UDamageType::StaticClass();
	}
};

// A weapon that fires a visible projectile
UCLASS(Abstract)
class FUSION_API AFusionWeapon_Projectile : public AMasterWeapon
{
	GENERATED_BODY()

	AFusionWeapon_Projectile(const FObjectInitializer& ObjectInitializer);

protected:



	virtual EAmmoType GetAmmoType() const override { return EAmmoType::ERocket; }

	/** weapon config */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	FProjectileWeaponData ProjectileConfig;

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() override;

	/** spawn projectile on server */
	UFUNCTION(reliable, server, WithValidation)
	void ServerFireProjectile(FVector Origin, FVector_NetQuantizeNormal ShootDir);
	void ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir);
	bool ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir);

public:
	/** apply config on projectile */
	void ApplyWeaponConfig(FProjectileWeaponData& Data);

};


	