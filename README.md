# Library structure

## bmsound-wine
This library acts as a bridge between windows<->unix libraries running inside wine environment.              
Makefile environment specific to wine applies (see [winebuild](https://www.winehq.org/docs/winebuild)).           

## bmsound-pw
This library runs fully in unix userspace and is not directly aware of wine environment.             
This library implements various handlers for translating bemani audio interface callbacks into ones compatible with pipewire's native API.                
Makefile environment native to unix applies.           

# Supported formats
Audio formats tested. Listed by lookup priority where applicable.

## IIDX
| Format              | Support  |
|:--------------------|:---------|
| 44.1kHz@32bit 7.1ch | &#10008; |
| 44.1kHz@24bit 7.1ch | &#10008; |
| 44.1kHz@16bit 7.1ch | &#10008; |
| 44.1kHz@32bit 5.1ch | &#10008; |
| 44.1kHz@16bit 7.1ch | &#10008; |
| 44.1kHz@32bit 3.1ch | &#10008; |
| 44.1kHz@16bit 3.1ch | &#10008; |
| 44.1kHz@16bit 2ch   | &#10004; |

# Building notes
* define build-wide `BMSW_NOEXPERIMENTAL` to build without additional test code/unstable APIs
* define build-wide `BMSW_NODBGEXEC` to disable compiling of any additional issue tracing code
* changes to `translation_unit.frag.c` will not trigger `translation_unit.o` rebuild, applying any change directly to `translation_unit.c` will
