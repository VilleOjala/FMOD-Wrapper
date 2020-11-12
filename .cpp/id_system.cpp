
#include "id_system.h"

unsigned long int IdSystem::id_incrementer = 0;

unsigned long int IdSystem::getUniqueId()
{	
	return ++id_incrementer;
}