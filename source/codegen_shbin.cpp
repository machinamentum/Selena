#include "codegen_shbin.h"

enum { SHADER_TYPE_VERTEX = 0, SHADER_TYPE_GEO = 1 };

struct __attribute__((packed)) dvlb {
  int Magic = 'D' | 'V' << 8 | 'L' << 16 | 'B' << 24;
  int DVLECount;
  int DVLEOffset;
};

struct __attribute__((packed)) dvlp {
  int Magic = 'D' | 'V' << 8 | 'L' << 16 | 'P' << 24;
  int Unk = 0;
  int ShaderBlobOffset;
  int ShaderBlobSize;
  int ShaderInstructionExtTableOffset;
  int ShaderInstructionExtCount;
  int SymbolTableOffset;
};

struct __attribute__((packed)) dvle {
  int Magic = 'D' | 'V' << 8 | 'L' << 16 | 'E' << 24;
  short Pad2 = 0;
  char ShaderType;
  char Unk = 0;
  int ExecEntryOffset;
  int ExecEntryEndOffset;
  int Pad0 = 0;
  int Pad1 = 0;
  int ConstantTableOffset;
  int ConstantCount;
  int LabelTableOffset;
  int LabelCount;
  int OutputRegTableOffset;
  int OutputRegCount;
  int UniformRegTableOffset;
  int UniformRegCount;
  int SymbolTableOffset;
  int SymbolTableSize;
};

struct __attribute__((packed)) label_entry {
  char Id;
  char Unk[3] = {0, 0, 0};
  int BlobOffset;
  int Unk2 = 0;
  int SymbolOffset;
};

enum { CONST_TYPE_BOOL = 0, CONST_TYPE_IVEC4 = 1, CONST_TYPE_VEC4 = 2 };

struct __attribute__((packed)) const_entry {
  char Type;
  char Pad0 = 0;
  char Id;
  char Pad1 = 0;
  int X;
  int Y;
  int Z;
  int W;
};

struct __attribute__((packed)) uniform_entry {
  int SymbolOffset;
  short StartReg;
  short EndReg;
};

struct __attribute__((packed)) op_desc_entry {
  int Swizzle;
  int Pad;
};

struct __attribute__((packed)) output_entry {
  char Type;
  char Pad0 = 0;
  char Register;
  char Pad1 = 0;
  int Mask;
};

struct shbin_gen {
  std::vector<std::string> SymbolTable;
  std::vector<label_entry> LabelTable;
  std::vector<uniform_entry> UniformTable;
  std::vector<const_entry> ConstTable;
  std::vector<op_desc_entry> OpDescTable;
  std::vector<output_entry> OutputTable;
  std::vector<unsigned int> Blob;
  dvlp DVLP;
  dvlb DVLB;
  dvle DVLE;
  neocode_program *Program;

  void GenBlob();
  void GenSymbolTable();
  void WriteShbin(std::ostream &os);
  int GenInstruction(neocode_instruction *Instruction);
  int GetSymbolOffset(const std::string &s);
  void GenConstTable();
  void GenUniformTable();
  void GenOutputTable();
};

#define OP_DESC(dst, src1, src2, src3)                                         \
  ((dst & 0b1111) | ((src1 & 0b111111111) << 4) |                              \
   ((src2 & 0b111111111) << 0xD) | ((src3 & 0b111111111) << 0x17))

#define INSTR_1(op, desc, dst, src1, src2, idx)                                \
  ((desc & 0b1111111) | ((src2 & 0b11111) << 0x7) |                            \
   ((src1 & 0b1111111) << 0xC) | ((idx & 0b11) << 0x13) |                      \
   ((dst & 0b11111) << 0x15) | ((op & 0b111111) << 0x1A))

#define INSTR_1U(op, desc, dst, src1, idx)                                     \
  ((desc & 0b1111111) | ((src1 & 0b1111111) << 0xC) | ((idx & 0b11) << 0x13) | \
   ((dst & 0b11111) << 0x15) | ((op & 0b111111) << 0x1A))

int shbin_gen::GenInstruction(neocode_instruction *Instruction) {
  if (Instruction->Type == neocode_instruction::EMPTY)
    return -1;
  int DstMask = 0;
  int Src1Comp = 0;
  int Src2Comp = 0;
  if (Instruction->Src1.TypeName.compare("mat4") == 0) {
    Src1Comp = 0b000110110;
  } else {
    int Swizz = Instruction->Src1.Swizzle;
    if (Swizz == 0) {
      Src1Comp = 0b000110110;
    } else {
      for (int i = 0; i < 4; ++i) {
        int V = ((Swizz >> (i * 4)) & 0b1111) - 1;
        if (V < 0)
          continue;
        Src1Comp |= (V << ((0x6 - (i * 2)) + 1));
      }
    }
  }
  {
    int Swizz = Instruction->Src2.Swizzle;
    if (Swizz == 0) {
      Src2Comp = 0b000110110;
    } else {
      for (int i = 0; i < 4; ++i) {
        int V = ((Swizz >> (i * 4)) & 0b1111) - 1;
        if (V < 0)
          continue;
        Src2Comp |= (V << ((0x6 - (i * 2)) + 1));
      }
    }
  }
  int DstSwizz = Instruction->Dst.Swizzle;
  if (DstSwizz == 0) {
    DstMask = 0b1111;
  } else {
    for (int i = 0; i < 4; ++i) {
      int V = ((DstSwizz >> (i * 4)) & 0b1111) - 1;
      if (V < 0)
        continue;
      DstMask |= (1 << (3 - V));
    }
  }

  int OpDesc = OP_DESC(DstMask, Src1Comp, Src2Comp, 0);
  int OpDescIndex = -1;
  for (int i = 0; i < OpDescTable.size(); ++i) {
    op_desc_entry &e = OpDescTable[i];
    if (e.Swizzle == OpDesc) {
      OpDescIndex = i;
      break;
    }
  }
  if (OpDescIndex < 0) {
    OpDescTable.push_back((op_desc_entry){OpDesc, 0});
    OpDescIndex = OpDescTable.size() - 1;
  }
  int Src1Reg = Instruction->Src1.Register +
                (Instruction->Src1.TypeName.compare("mat4") == 0
                     ? Instruction->Src1.Swizzle
                     : 0);
  switch (Instruction->Type) {
  case neocode_instruction::MOV:
    return INSTR_1U(0x13, OpDescIndex, Instruction->Dst.Register, Src1Reg, 0);

  case neocode_instruction::DP4:
    return INSTR_1(0x02, OpDescIndex, Instruction->Dst.Register,
                   Src1Reg, Instruction->Src2.Register, 0);

  case neocode_instruction::MUL:
    return INSTR_1(0x08, OpDescIndex, Instruction->Dst.Register, Src1Reg,
                   Instruction->Src2.Register, 0);

  case neocode_instruction::RSQ:
    return INSTR_1U(0x0F, OpDescIndex, Instruction->Dst.Register, Src1Reg, 0);

  case neocode_instruction::RCP:
    return INSTR_1U(0x0E, OpDescIndex, Instruction->Dst.Register, Src1Reg, 0);

  case neocode_instruction::NOP:
    return 0x21 << 0x1A;

  case neocode_instruction::END:
    return 0x22 << 0x1A;

  case neocode_instruction::EX2:
    return INSTR_1U(0x05, OpDescIndex, Instruction->Dst.Register, Src1Reg, 0);

  case neocode_instruction::LG2:
    return INSTR_1U(0x06, OpDescIndex, Instruction->Dst.Register, Src1Reg, 0);

  default:
    return -1;
  }
}

int shbin_gen::GetSymbolOffset(const std::string &s) {
  int Off = 0;
  for (std::string &str : SymbolTable) {
    if (str.compare(s) == 0) {
      return Off;
    }
    Off += str.length() + 1;
  }

  return 0;
}

void shbin_gen::GenBlob() {
  for (neocode_function &F : Program->Functions) {
    if (F.Name.compare("main") == 0) {
      DVLE.ExecEntryOffset = Blob.size();
    }
    label_entry E;
    E.Id = 0;
    // E.Id = LabelTable.size();
    E.BlobOffset = Blob.size();
    E.SymbolOffset = GetSymbolOffset(F.Name);
    LabelTable.push_back(E);
    for (neocode_instruction &Instruction : F.Instructions) {
      int Instr = GenInstruction(&Instruction);
      if (Instr != -1)
        Blob.push_back(Instr);
    }
    // E.Id = LabelTable.size();
    E.BlobOffset = Blob.size();
    E.SymbolOffset = GetSymbolOffset(F.Name + "_end");
    LabelTable.push_back(E);
    if (F.Name.compare("main") == 0) {
      // DVLE.ExecEntryEndOffset = Blob.size();
    }
  }
}

void shbin_gen::GenSymbolTable() {
  for (neocode_function &F : Program->Functions) {
    SymbolTable.push_back(F.Name);
    SymbolTable.push_back(F.Name + "_end");
  }

  for (neocode_variable &V : Program->Globals) {
    if ((V.Register < 0x20 && V.RegisterType <= 0) || (V.RegisterType < 0))
      SymbolTable.push_back(V.Name);
  }
}

void shbin_gen::GenOutputTable() {
  for (neocode_variable &V : Program->Globals) {
    if (V.RegisterType > 0) {
      output_entry E;
      E.Type = V.RegisterType - 1;
      E.Register = V.Register;
      E.Mask = 0b1111;
      OutputTable.push_back(E);
    }
  }
}

void shbin_gen::GenUniformTable() {
  for (neocode_variable &V : Program->Globals) {
    if ((V.Register < 0x20 && V.RegisterType <= 0) || (V.RegisterType < 0)) {
      uniform_entry E;
      E.SymbolOffset = GetSymbolOffset(V.Name);
      E.StartReg = E.EndReg =
          (V.Register >= 0x20 ? V.Register - 0x10 : V.Register);
      if (V.TypeName.compare("mat4") == 0) {
        E.EndReg = E.StartReg + 3;
      }
      UniformTable.push_back(E);
    }
  }
}

static unsigned int f32tof24(float f) {
  unsigned int i = *(unsigned int *)&f;

  unsigned int mantissa = (i << 9) >> 9;
  int exponent = (i << 1) >> 24;
  unsigned int sign = (i << 0) >> 31;

  mantissa >>= 7;
  exponent = exponent - 127 + 63;
  if (exponent < 0)
    return sign << 23;
  else if (exponent > 0x7F)
    return sign << 23 | 0x7F << 16;

  return sign << 23 | exponent << 16 | mantissa;
}

void shbin_gen::GenConstTable() {
  {
    const_entry E;
    E.Type = 2;
    E.Id = 95;
    E.X = f32tof24(0.0);
    E.Y = f32tof24(0.0);
    E.Z = f32tof24(0.0);
    E.W = f32tof24(0.1);
    ConstTable.push_back(E);
  }
  for (neocode_variable &V : Program->Globals) {
    if (V.RegisterType == 0 && V.Register >= 0x20) {
      const_entry E;
      // TODO bool, ivec4
      E.Type = 2;
      E.Id = V.Register - 0x20;
      E.X = f32tof24(V.Const.Float.X);
      E.Y = f32tof24(V.Const.Float.Y);
      E.Z = f32tof24(V.Const.Float.Z);
      E.W = f32tof24(V.Const.Float.W);
      ConstTable.push_back(E);
    }
  }
}

void shbin_gen::WriteShbin(std::ostream &os) {
  os.write((char *)&DVLB, sizeof(dvlb));
  os.write((char *)&DVLP, sizeof(dvlp));
  os.write((char *)&DVLE, sizeof(dvle));
  for (int i : Blob) {
    os.write((char *)&i, sizeof(i));
  }
  for (op_desc_entry &e : OpDescTable) {
    os.write((char *)&e, sizeof(op_desc_entry));
  }
  for (output_entry &e : OutputTable) {
    os.write((char *)&e, sizeof(output_entry));
  }

  for (const_entry &e : ConstTable) {
    os.write((char *)&e, sizeof(const_entry));
  }
  for (uniform_entry &e : UniformTable) {
    os.write((char *)&e, sizeof(uniform_entry));
  }
  for (label_entry &e : LabelTable) {
    os.write((char *)&e, sizeof(label_entry));
  }
  for (std::string &s : SymbolTable) {
    os.write(s.c_str(), s.length() + 1);
  }
}

void CGShbinGenerateCode(neocode_program *Program, std::ostream &os) {
  shbin_gen Shbin;
  Shbin.Program = Program;
  Shbin.GenSymbolTable();
  Shbin.GenConstTable();
  Shbin.GenUniformTable();
  Shbin.GenOutputTable();
  Shbin.GenBlob();
  Shbin.DVLB.DVLECount = 1;
  Shbin.DVLB.DVLEOffset = sizeof(dvlb) + sizeof(dvlp);
  Shbin.DVLP.ShaderBlobOffset = sizeof(dvlp) + sizeof(dvle);
  Shbin.DVLP.ShaderBlobSize = Shbin.Blob.size();
  Shbin.DVLP.ShaderInstructionExtTableOffset =
      sizeof(dvlp) + sizeof(dvle) + Shbin.Blob.size() * 4;
  Shbin.DVLP.ShaderInstructionExtCount = Shbin.OpDescTable.size();
  Shbin.DVLP.SymbolTableOffset = 0;

  Shbin.DVLE.ShaderType = 0;
  Shbin.DVLE.ConstantTableOffset =
      sizeof(dvle) + Shbin.Blob.size() * 4 +
      Shbin.OpDescTable.size() * sizeof(op_desc_entry) +
      Shbin.OutputTable.size() * sizeof(output_entry);
  Shbin.DVLE.ConstantCount = Shbin.ConstTable.size();
  Shbin.DVLE.LabelTableOffset =
      sizeof(dvle) + Shbin.Blob.size() * 4 +
      Shbin.OpDescTable.size() * sizeof(op_desc_entry) +
      Shbin.OutputTable.size() * sizeof(output_entry) +
      Shbin.ConstTable.size() * sizeof(const_entry) +
      Shbin.UniformTable.size() * sizeof(uniform_entry);
  Shbin.DVLE.LabelCount = Shbin.LabelTable.size();
  Shbin.DVLE.OutputRegTableOffset =
      sizeof(dvle) + Shbin.Blob.size() * 4 +
      Shbin.OpDescTable.size() * sizeof(op_desc_entry);
  Shbin.DVLE.OutputRegCount = Shbin.OutputTable.size();
  Shbin.DVLE.UniformRegTableOffset =
      sizeof(dvle) + Shbin.Blob.size() * 4 +
      Shbin.OpDescTable.size() * sizeof(op_desc_entry) +
      Shbin.OutputTable.size() * sizeof(output_entry) +
      Shbin.ConstTable.size() * sizeof(const_entry);
  Shbin.DVLE.UniformRegCount = Shbin.UniformTable.size();
  Shbin.DVLE.SymbolTableOffset =
      sizeof(dvle) + Shbin.Blob.size() * 4 +
      Shbin.OpDescTable.size() * sizeof(op_desc_entry) +
      Shbin.OutputTable.size() * sizeof(output_entry) +
      Shbin.ConstTable.size() * sizeof(const_entry) +
      Shbin.UniformTable.size() * sizeof(uniform_entry) +
      Shbin.LabelTable.size() * sizeof(label_entry);
  Shbin.DVLE.SymbolTableSize = 0;
  for (std::string &s : Shbin.SymbolTable) {
    Shbin.DVLE.SymbolTableSize += s.length() + 1;
  }
  Shbin.WriteShbin(os);
}
