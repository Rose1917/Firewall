cmd_/home/march1917/Projects/firewall/kernel.ko := ld -r -m elf_x86_64  -z max-page-size=0x200000  --build-id  -T ./scripts/module-common.lds -o /home/march1917/Projects/firewall/kernel.ko /home/march1917/Projects/firewall/kernel.o /home/march1917/Projects/firewall/kernel.mod.o;  true