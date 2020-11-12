MIT License
Copyright (c) 2020 Ville Ojala

#pragma once

class IdSystem
{
private:

	friend class FmodWrapper;

	static unsigned long int id_incrementer; 

public:

	unsigned long int getUniqueId();
};
