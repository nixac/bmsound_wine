# Library structure

## bmsound-wine
This library acts as a bridge between windows<->unix libraries running inside wine environment.
Makefile environment specific to wine applies (see [winebuild](https://www.winehq.org/docs/winebuild)).

## bmsound-pw
This library runs fully in unix userspace and is not directly aware of wine environment.
This library implements various handlers for translating bemani audio interface callbacks into ones compatible with pipewire's native API.
Makefile environment native to unix applies.
