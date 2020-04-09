/*
 * lookup.h
 *
 * Created: 5. 04. 2020 14:23:35
 *  Author: matjaz tome
 */ 


#ifndef LOOKUP_H_
#define LOOKUP_H_

#include <stdint.h>
#include <avr/pgmspace.h>

typedef enum 
{
	LOCATION_RAM,
	LOCATION_FLASH
}LookupTableLoc_t;

typedef struct LookupTableStruct{
	LookupTableLoc_t location; // ali je tabela v RAMu ali FLASHU
	uint8_t step;	// korak vrednosti med sosednjima x vrednostma
	uint8_t table_size;	//velikost tabele
	uint16_t x_min;		// najnižja vrednsot v lookup tabeli
	int16_t *p_table;	//Kazalec na tabelo v flashu ali ramu
} lookup_table_t;

int16_t Lookup( uint16_t x_value, lookup_table_t *tabela);
void Lookup_init (lookup_table_t* table, LookupTableLoc_t location, uint8_t step, uint16_t xMin, uint8_t size, int16_t* tabel_loc);



#endif /* LOOKUP_H_ */