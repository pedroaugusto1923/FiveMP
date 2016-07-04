#pragma once

class GameWeapon {
public:
	void GiveWeapon(char * weaponid, int ammo);
	void RemoveWeapon(char * weaponid);
	void GiveAmmo(char * weaponid, int ammo);
	void RemoveAmmo(char * weaponid, int ammo);
};

