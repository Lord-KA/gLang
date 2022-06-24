#include <elf.h>
#include <stdio.h>
#include <assert.h>

/* progr load virtual offset to entry point */
static const uint32_t CODE_OFFSET_ = 0x80;

/* progr load virtual addres */
static const uint64_t LOAD_VIRT_ADDR_ = 0x400000;

//////////////////////////////////////////////////////////////
//                          My shit!                        //
//////////////////////////////////////////////////////////////

Elf64_Ehdr elf_hdr_setup()
{
    Elf64_Ehdr h = {};

    h.e_ident[0] = 0x7F;                /* ELF identification */
    h.e_ident[1] = 'E';
    h.e_ident[2] = 'L';
    h.e_ident[3] = 'F';
    h.e_ident[EI_CLASS]   = ELFCLASS64;
    h.e_ident[EI_DATA]    = ELFDATA2LSB;
    h.e_ident[EI_VERSION] = EV_CURRENT;
    h.e_ident[EI_OSABI]   = ELFOSABI_NONE;
    h.e_ident[EI_ABIVERSION] = 0x00;

    h.e_type = ET_EXEC;                         /* Object file type */
    h.e_machine = EM_X86_64;                    /* Machine type */
    h.e_version = EV_CURRENT;                   /* Object file version */
    h.e_entry = LOAD_VIRT_ADDR_ + CODE_OFFSET_; /* Entry point address */
    fprintf(stderr, "entry = %zu\n", h.e_entry);
    h.e_phoff = 0x40;                           /* Program header offset */
    h.e_shoff = 0x00;                           /* Section header offset */
    h.e_flags = 0x00;                           /* Processor-specific flags */
    h.e_ehsize    = 0x40;                       /* ELF header size */
    h.e_phentsize = 0x38;                       /* Size of program header entry */
    h.e_phnum     = 0x01;                       /* Number of program header entries */
    h.e_shentsize = 0x40;                       /* Size of section header entry */
    h.e_shnum     = 0x00;                       /* Number of section header entries */
    h.e_shstrndx  = 0x00;                       /* Section name string table index */

    return h;
}

static const uint8_t program[] = {
    0x50,
    0x49, 0xba, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x41, 0xff, 0xd2,
    0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00, // mov rax, 60
    0x48, 0xc7, 0xc7, 0x00, 0x00, 0x00, 0x00, // mov rdi, 0
    0x0f, 0x05                                // syscall
};


Elf64_Phdr progr_hdr_setup(size_t prog_size)
{
    Elf64_Phdr h = {};

    h.p_type = PT_LOAD;                             /* Type of segment */
    h.p_flags  = PF_R | PF_X | PF_W;                /* Segment attributes */
    h.p_offset = CODE_OFFSET_;                      /* Offset in file */
    h.p_vaddr = LOAD_VIRT_ADDR_ + CODE_OFFSET_;     /* Virtual address in memory */
    h.p_paddr = LOAD_VIRT_ADDR_ + CODE_OFFSET_;     /* Reserved */
    h.p_filesz = prog_size;                         /* Size of segment in file */
    h.p_memsz  = prog_size;                         /* Size of segment in memory */
    h.p_align  = 0x0001;                            /* Alignment of segment */

    return h;
}

int main()
{
    FILE *out = fopen("test.out", "w");
    assert(out != NULL);

    Elf64_Ehdr ehdr = elf_hdr_setup();
    Elf64_Phdr phdr = progr_hdr_setup(sizeof(program));
    fwrite(&ehdr, sizeof(ehdr), 1, out);
    fwrite(&phdr, sizeof(phdr), 1, out);
    const size_t offset_len = CODE_OFFSET_ - sizeof(ehdr) - sizeof(phdr);
    uint8_t offset[offset_len] = {};
    fwrite(offset,  sizeof(uint8_t), offset_len,  out);
    fwrite(program, sizeof(uint8_t), sizeof(program), out);
    fclose(out);
}
