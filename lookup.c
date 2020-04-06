/*
 * lookup.c
 *
 * Created: 5. 04. 2020 14:23:46
 *  Author: matjaz tome
 */ 

#include "lookup.h"



//************************************
// Method:    Lookup
// FullName:  Lookup
// Access:    public
// Returns:   uint_16t
// Qualifier: Pogleda v tabelo in vrne linearno interpolirano vrednosti izmed dvema sosednjima vrednostma, korak x tabele je konstanten in monotono naraš?ujo?
// Parameter: uint16_t x_value je spremenljivka, ki dolo?a lokacijo v tabeli
// Parameter: LookupTableStruct_t *tabela je pointer na strukturo, ki dolo?a vse lastnosti tabele
//*
uint16_t Lookup( uint16_t x_value, lookup_table_t *tabela)
{
	uint16_t element_index;
	uint16_t delta;
	int16_t k;
	uint16_t x_norm; // so we can calulate (x_value - tabela->x_min) only once
	
	// Calculate x_norm - speed optimization
	x_norm = (x_value - tabela->x_min);
	
	//check if x data lies in table
	if(x_norm > (tabela->indeks_korak * (tabela->table_size - 1)))
	{
		return 0; // this means error
	}
	if(x_value < tabela->x_min)
	{
		return 0;
	}
	
	//find the closest smaller element in table_size
	element_index = x_norm / tabela->indeks_korak;
	if(element_index == tabela->table_size - 1) // this means we are accesing last elemet in table. Just return it's value, otherwise [element_index + 1] will fail
	{
		if(tabela->location == LOCATION_RAM)
		{
			return tabela->p_table[element_index];
		}
		else
		{
			return pgm_read_word(&(tabela->p_table[element_index]));
		}
		
	}
	
	//calcualte where between two table values lies our x_value
	delta = x_norm - (element_index * tabela->indeks_korak);
	
	//calulate steepnes of curve between our table points
	if(tabela->location == LOCATION_RAM)
	{
		k = ((int16_t)tabela->p_table[element_index + 1] - (int16_t)tabela->p_table[element_index]) / tabela->indeks_korak;
	}
	else
	{
		k = ((int16_t)pgm_read_word(&(tabela->p_table[element_index + 1])) - (int16_t)pgm_read_word(&(tabela->p_table[element_index]))) / tabela->indeks_korak;
	}
	
	
	// calulate final value
	if(tabela->location == LOCATION_RAM)
	{
		return (tabela->p_table[element_index]+ k * delta);
	}
	else
	{
		return (pgm_read_word(&tabela->p_table[element_index]) + k * delta);
	}
	
}


//************************************
// Method:    Lookup_init
// FullName:  Lookup_init
// Access:    public
// Returns:   void
// Qualifier: Pripravi strukturo za lookup z ustreznimi parametri
// Parameter: lookup_table_t* table - kazalec na strukturo ki jo želimo konfigurirati
// Parameter: LookupTableLoc_t location - parameter pove ali je tabela v RAM-u (LOCATION_RAM) ali flashu (LOCATION_FLAH)
// Parameter: uint8_t step -  korak vrednosti med sosednjima x vrednostma
// Parameter: uint8_t size - velikost tabele
// Parameter: uint16_t* tabel_loc - kazalec na tabelo
//*
void Lookup_init (lookup_table_t* table, LookupTableLoc_t location, uint8_t step, uint16_t xMin, uint8_t size, uint16_t* tabel_loc)
{
	table->location = location;
	table->indeks_korak = step;
	table->x_min = xMin;
	table->table_size = size;
	table->p_table = tabel_loc;
}