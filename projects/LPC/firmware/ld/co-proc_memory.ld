MEMORY
{
  /* Define each memory region */
  RamAHB16 (rwx)     : ORIGIN = 0x20008000, LENGTH = 0x4000 /* 16K bytes (alias RAM) -- primary RAM (data & stack), 
                                                               and code and inititalized data image, will be uploaded by M4 */
  RamAHB_ETB16 (rwx) : ORIGIN = 0x2000c000, LENGTH = 0x4000 /* 16K bytes (alias RAM2) -- shared between M0 and M4 */  
}

  /* Define a symbol for the top of each memory region */
  __base_RamAHB16 = 0x20008000  ; /* RamAHB16 */  
  __base_RAM = 0x20008000 ; /* RAM */  
  __top_RamAHB16 = 0x20008000 + 0x4000 ; /* 16K bytes */  
  __top_RAM = 0x20008000 + 0x4000 ; /* 16K bytes */  
  __base_RamAHB_ETB16 = 0x2000c000  ; /* RamAHB_ETB16 */  
  __base_RAM2 = 0x2000c000 ; /* RAM2 */  
  __top_RamAHB_ETB16 = 0x2000c000 + 0x4000 ; /* 16K bytes */  
  __top_RAM2 = 0x2000c000 + 0x4000 ; /* 16K bytes */  
