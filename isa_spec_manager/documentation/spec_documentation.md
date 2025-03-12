# AMD XML ISA Specification
## Motivation
This documentation gives a detailed description of AMD’s machine-readable GPU instruction set architecture specification.

## Machine-readable ISA specification
This specification is specifically designed to be easily and efficiently read by a computer program.

The top-level XML schema is divided into two main elements:
* Document: contains metadata about the current specification XML file.
* ISA: describes the Instruction Set Architecture itself for the relevant GPU architecture.

Here is a top-level view of the schema:
```
<Spec>
    <Document>
    <ISA> 
        <Architecture>
        <Instructions>
        <Encodings>
        <DataFormats>
        <OperandTypes>
        <FunctionalGroups>
```

A detailed description of each of the XML elements in the schema is provided below.

## AMD GPUs XML ISA Specification
In this section, we will take a deep dive into the XML specification and break down every single element. The description of elements will be in a breadth first order. Every subsection will describe a single element by stating its parent element and list of child elements.

### \<Spec\>
Hierarchy: **\<Spec\>**

Description: top level element which holds information about the ISA specification.

List of child elements:
| #  | Element                      | XML element name | Description |
| -- | ---------------------------- | ---------------- | - |
| 1. | Document                     | [\<Document\>](#document) | XML document related information. |
| 2. | Instruction Set Architecture | [\<ISA\>](#isa)           | ISA related information. |

### \<Document\> 
Hierarchy: \<Spec\> → **\<Document\>**

Description: encapsulates generic information about the XML document.

List of child elements:
| #  | Element        | XML element name | Description|
| -- | -------------- | ---------------- | - |
| 1. | Copyright      | \<Copyright\>      | Copyright notice. |
| 2. | Sensitivity    | \<Sensitivity\>    | Restriction level of the document. For example, the XML specification could be distributed publicly, with NDA, or in rare cases restricted to internal usage only. |
| 3. | Release date   | \<ReleaseDate\>    | The date when the specification was released. |
| 4. | Schema version | \<SchemaVersion\>  | The version of the schema. |

### \<ISA\>
Hierarchy: \<Spec\> → **\<ISA\>**

Description: contains ISA related information.

List of child elements:
| #  | Element       | XML element name | Description|
| - | ------------- | ---------------- | - |
| 1. | Architecture  | [\<Architecture\>](#architecture) | Architecture-specific meta data. |
| 2. | Instructions  | [\<Instructions\>](#instructions) | Lists all instructions in the architecture. This is the core element of the specification. Every instruction references other XML elements. Examples of provided information by this element: different ways to encode the instruction, the type of operands, the data format of the operand, etc. |
| 3. | Encodings     | [\<Encodings\>](#encodings)       | Lists all encodings supported by the architecture. Examples of the provided information by this element: instruction sizes, fields of the binary instruction, general description of the encoding. |
| 4. | Data formats  | [\<DataFormats\>](#dataformats)   | Lists all data formats in the architecture. It provides additional information on how the values in the registers should be treated. This element is referenced by the instruction element. Examples of provided information by this element: is the value integer or float? If it is float where is the mantissa, exponent and sign? |
| 5. | Operand types | [\<OperandTypes\>](#operandtypes) | Lists all operand types in the architecture. The sub-elements of this element are referenced by an instruction element. It provides information on the types of the operands used by the instruction. Examples of provided information are: is the operand a scalar or a vector register? What is the name of this operand when represented in assembly?|
| 6. | Functional groups | [\<FunctionalGroups\>](#functionalgroups) | Lists all functional groups in the architecture. A function group provides high-level classification of the instructions, such as: vector memory, vector ALU, scalar memory, etc. |

### \<Architecture\>
Hierarchy: \<Spec\> → \<ISA\> → **\<Architecture\>**

Description: contains information about architecture specific meta data.

List of child elements:
| #  | Element           | XML element name   | Description |
| - | ----------------- | ------------------ | - |
| 1. | Architecture name | \<ArchitectureName\> | The name of the architecture described in the XML.|

---

### \<Instructions\>
Hierarchy: \<Spec\> → \<ISA\> → **\<Instructions\>**

Description: lists all instructions in the architecture. This is the core element of the specification. Every instruction references other XML elements. Examples of provided information by this element: different ways to encode the instruction, the type of operands, the data format of the operand, etc.

List of child elements:
| #  | Element                | XML element name | Description|
| -- | ---------------------- | ---------------- | - |
| 1. | Instruction (singular) | [\<Instruction\>](#instruction)    | Information about a single instruction in this architecture.|

### \<Instruction\>
Hierarchy: \<Spec\> → \<ISA\> → \<Instructions\> → **\<Instruction\>**

Description: information about a single instruction in this architecture, including its name, operation, and associated parameters.

List of child elements:
| #  | Element               | XML element name       | Description |
| -- | --------------------- | ---------------------- | - |
| 1. | Instruction flags     | [\<InstructionFlags\>](#instructionflags)     | Information about basic features of the instruction. For example, is the instruction a branch? Does it terminate the program? |
| 2. | Instruction name      | \<InstructionName\>      | The name of the instruction. This is the assembly name of the instruction.|
| 3. | Description           | \<Description\>          | Description of the operation performed by the instruction. |
| 4. | Instruction encodings | [\<InstructionEncodings\>](#instructionencodings) | Lists all encodings this instruction can be represented in. |

### \<InstructionFlags\>
Hierarchy: \<Spec\> → \<ISA\> → \<Instructions\> → \<Instruction\> → **\<InstructionFlags\>**

Description: information about basic features of the instruction. For example, is the instruction a branch? Does it terminate the program?

List of child elements:
| #  | Element                 | XML element name        | Description|
| -- | ----------------------- | ----------------------- | -|
| 1. | Is branch               | \<IsBranch\>              | `True` if the instruction is a branch, `False` otherwise. |
| 2. | Is program terminator   | \<IsProgramTerminator\>   | `True` if the instruction is a program terminator, `False` otherwise. |
| 3. | Is immediately executed | \<IsImmediatelyExecuted\> | `True` if the instruction does NOT go through the execution unit, `False` otherwise. |

### \<InstructionEncodings\>
Hierarchy: \<Spec\> → \<ISA\> → \<Instructions\> → \<Instruction\> → **\<InstructionEncodings\>**

Description: lists all encodings this instruction can be represented in. 

List of child elements:
| #  | Element              | XML element name      | Description |
| -- | -------------------- | --------------------- | - |
| 1. | Instruction encoding (singular) | [\<InstructionEncoding\>](#instructionencoding) |  One possible encoding version given instruction can be represented in. |

### \<InstructionEncoding\>
Hierarchy: \<Spec\> → \<ISA\> → \<Instructions\> → \<Instruction\> → \<InstructionEncodings\> → **\<InstructionEncoding\>**

Description: one possible encoding version given instruction can be represented in. 

List of child elements:
| #  | Element       | XML element name | Description|
| -- | ------------- | ---------------- | - |
| 1. | Encoding name | \<EncodingName\>   | The encoding name the given instruction is represented in. Should match one of the names in the list of [\<Encodings\>](#encodings)|
| 2. | EncodingCondition | \<ConditionName\> | Name of the condition that must be true to select this encoding. |
| 2. | Opcode        | \<Opcode\>         | The opcode value of the instruction when represented in this encoding. |
| 3. | Operands      | [\<Operands\>](#operands) | Lists all operands of the instruction when represented in this encoding. |

### \<Operands\>
Hierarchy: \<Spec\> → \<ISA\> → \<Instructions\> → \<Instruction\> → \<InstructionEncodings\> → \<InstructionEncoding\>  → **\<Operands\>**

Description: lists all operands of the instruction when represented in this encoding.

List of child elements:
| #  | Element | XML element name | Description|
| -- | ------- | ---------------- | - |
| 1. | Operand (singular) | [\<Operand\>](#operand) | Information about a single operand in the instruction. |

### \<Operand\>
Hierarchy: \<Spec\> → \<ISA\> → \<Instructions\> → \<Instruction\> → \<InstructionEncodings\> → \<InstructionEncoding\>  → \<Operands\>  → **\<Operand\>**

Description: Information about a single operand in the instruction.

List of child elements:
| #  | Element          | XML element name | Description                                                                                                   |
| -- | ---------------- | ---------------- | - |
| 1. | Field name       | \<FieldName\>      | Encoding field name. Should match one of the fields in [\<BitMap\>](#bitmap-microcodeformat). |
| 2. | Data format name | \<DataFormatName\> | Data format of the operand. Should match one of the formats in [\<DataFormats\>](#dataformats). |
| 3. | Operand type     | \<OperantType\>    | Type of the operand. Should match one of the types in [\<OperandTypes\>](#operandtypes)|
| 4. | Operand size     | \<OperandSize\>    | The size of this operand in bits. |


List of attributes:
| #  | Attribute                    | Attribute name            | Description|
| -- | ---------------------------- | ------------------------- | - |
| 1. | Is operand source            | Input                     | `True` if the operand is read from, `False` otherwise. |
| 2. | Is operand destination       | Output                    | `True` if the operand is written to, `False` otherwise. |
| 3. | Is operand implicit          | IsImplicit                | `True` if the operand is missing from the encoding and implied by the instruction. If true the operand is not shown in assembly. |
| 4. | Is binary microcode required | IsBinaryMicrocodeRequired | `True` if the operand is missing from the encoding and implied by the operand type. If true the operand is missing in the encoding but is present in the assembly. |

---

### \<Encodings\>
Hierarchy: \<Spec\> → \<ISA\> → **\<Encodings\>**

Description: lists all encodings supported by the architecture. Examples of the provided information by this element: instruction sizes, fields of the binary instruction, general description of the encoding.

List of child elements:
|#  | Element             | XML element name | Description |
|-- | ------------------- | ---------------- | - |
|1. | Encoding (singular) | [\<Encoding\>](#encoding) | Information about a single encoding in this architecture. |

### \<Encoding\>
Hierarchy: \<Spec\> → \<ISA\> → \<Encodings\> → **\<Encoding\>**

Description: information about a single encoding in this architecture.

List of child elements:
| #  | Element                  | XML element name         | Description|
| -- | ------------------------ | ------------------------ | - |
| 1. | Encoding name            | \<EncodingName\>           | Encoding name. For example, VOP1, SOP1, etc. |
| 2. | Bit count                | \<BitCount\>               | Size of the encoding in bits. |
| 3. | Encoding identifier mask | \<EncodingIdentifierMask\> | Binary mask to identify the encoding. |
| 4. | Encoding identifiers     | [\<EncodingIdentifiers\>](#encodingidentifiers)  | Lists all unique identifiers which are used to map to the encoding. |
| 5. | Encoding conditions      | [\<EncodingConditions\>](#encodingconditions)    | Lists all encoding conditions. A condition specifies cases when the encoding can be extended with extra fields such as literal constant, DPP, etc. |
| 6. | Description              | \<Description\>            | Encoding description. |
| 7. | Microcode format         | [\<MicrocodeFormat\> → \<BitMap\>](#bitmap-microcodeformat) | Information about binary break down of the encoding. |


List of attributes:
| #  | Attribute      | Attribute name | Description                                                                              |
| -- | -------------- | -------------- | - |
| 1. | Encoding order | Order          | Dictates the order in which encodings should be picked when decoding machine code, with the search halting at the first match of the identifier. |

### \<EncodingIdentifiers\>
Hierarchy: \<Spec\> → \<ISA\> → \<Encodings\> → \<Encoding\>  → **\<EncodingIdentifiers\>**

Description: lists all unique identifiers which are used to map to the encoding.

List of child elements:
| #  | Element             | XML element name     | Description|
| -- | ------------------- | -------------------- | -|
| 1. | Encoding Identifier (singular) | \<EncodingIdentifier\> | Single unique identifier used to map to the specific encoding. This is the unique identifier of the instruction. The identifier is the combination of the encoding and opcode bits. |

### \<EncodingConditions\>
Hierarchy: \<Spec\> → \<ISA\> → \<Encodings\> → \<Encoding\>  → **\<EncodingConditions\>**

Description: lists all encoding conditions. A condition specifies cases when the encoding can be extended with extra fields such as literal constant, DPP, etc.

List of child elements:
| #  | Element            | XML element name    | Description|
| -- | ------------------ | ------------------- | -|
| 1. | Encoding Condition (singular) | [\<EncodingCondition\>](#encodingcondition) | A single condition. |

### \<EncodingCondition\>
Hierarchy: \<Spec\> → \<ISA\> → \<Encodings\> → \<Encoding\>  → \<EncodingConditions\> → **\<EncodingCondition\>**

Description: a single condition from the list of conditions of the encoding. A condition specifies cases when the encoding can be extended with extra fields such as literal constant, DPP, etc.

List of child elements:
| #  | Element              | XML element name      | Description|
| -- | -------------------- | --------------------- | - |
| 1. | Condition name       | \<ConditionName\>       | Name of the condition. Used as a reference when decoding an instruction and determining which encoding to use.|
| 2. | Condition expression | \<ConditionExpression\> | An abstract syntax tree. The tree encodes a boolean expression, which if evaluated to true signals that the extended version of the encoding must be used.|

### \<BitMap\> (MicrocodeFormat)
Hierarchy: \<Spec\> → \<ISA\> → \<Encodings\> → \<Encoding\>  → \<MicrocodeFormat\> → **\<BitMap\>**

Description: a bitmap holds a detailed breakdown of the fields in the given encoding.

List of child elements:
| #  | Element | XML element name | Description |
| -- | ------- | ---------------- | - |
| 1. | Field   | [\<Field\>](#field-microcodeformat--bitmap) | A single field from the list of fields in the bitmap. The field specifies the raw binary instruction must be broken down and interpreted.|

### \<Field\> (MicrocodeFormat → BitMap)
Hierarchy: \<Spec\> → \<ISA\> → \<Encodings\> → \<Encoding\>  → \<MicrocodeFormat\> → \<BitMap\> → **\<Field\>**

Description: a single field from the list of fields in the bitmap. The field specifies the raw binary instruction must be broken down and interpreted.

List of child elements:
| #  | Element    | XML element name | Description|
| -- | ---------- | ---------------- | - |
| 1. | Field name | \<FieldName\>      | Name of the field.|
| 2. | Bit layout | \<BitLayout\>      | The element contains information about the location of the field within the encoding, such as bit count and bit offset.|

---

### \<DataFormats\> 
Hierarchy: \<Spec\> → \<ISA\> → **\<DataFormats\>**

Description: lists all data fromats in the architecture. It provides additional information on how the values in the registers should be treated. This element is referenced by the instruction element. Examples of provided information by this element: is the value integer or float? If it is float where is mantissa, exponent and sign?

List of child elements:
| #  | Element                | XML element name | Description|
| -- | ---------------------- | ---------------- | - |
| 1. | Data format (singular) | [\<DataFormat\>](#dataformat) | Information about a single data format in this architecture. |

### \<DataFormat\>
Hierarchy: \<Spec\> → \<ISA\> → \<DataFormats\> → **\<DataFormat\>**

Description: information about a single data format in this architecture including a data type (descriptor, bits, float), the number of components packed into the given bits, and additional attributes.

List of child elements:
| #  | Element          | XML element name | Description |
| -- | ---------------- | ---------------- | - |
| 1. | Data format name | \<DataFormatName\> | Data format name. |
| 2. | Description      | \<Description\>    | Data format description. |
| 3. | Data type        | \<DataType\>       | Data type. For example, descriptor, bits, float.|
| 4. | Component count  | \<ComponentCount\> | Indicates the number of components this format packs into the given bits if data is packed. |
| 5. | Data attributes  | [\<DataAttributes\> → \<BitMap\>](#bitmap-dataattributes) | Information about each component in the data format. |

### \<BitMap\> (DataAttributes)
Hierarchy: \<Spec\> → \<ISA\> → \<DataFormats\> →  \<DataFormat\>  →  \<DataAttributes\> → **\<BitMap\>**

Description: information about each component in the data format.

List of child elements:
| #  | Element | XML element name | Description|
| -- | ------- | ---------------- | - |
| 1. | Field   | [\<Field\>](#field-dataattributes--bitmap) | Describes a single compoent field of the data format. |

### \<Field\> (DataAttributes → BitMap)
Hierarchy: \<Spec\> → \<ISA\> → \<DataFormats\> →  \<DataFormat\>  →  \<DataAttributes\> →  \<BitMap\>  → **\<Field\>**

Description: the data format can be broken down into several fields. This element describes a single field from the list of fields in the bitmap.

List of child elements:
| #  | Element    | XML element name | Description|
| -- | ---------- | ---------------- | - |
| 1. | Field name | \<FieldName\>      | Name of the field. |
| 2. | Bit layout | \<BitLayout\>      | Information about the location of the field within the data format.|


List of attributes:
| #  | Attribute        | Attribute name | Description |
| -- | ---------------- | -------------- | - |
| 1. | Field signedness | Signedness     | Determines if the field is signed or unsigned.|

---

### \<OperandTypes\>
Hierarchy: \<Spec\> → \<ISA\> → **\<OperandTypes\>**

Description: the sub-elements of this element are referenced by an instruction element. It provides information on the types of the operands used by the instruction. Examples of provided information are: is the operand a scalar or a vector register? What is the name of this operand when represented in assembly?

List of child elements:
| #  | Element                 | XML element name | Description|
| -- | ----------------------- | ---------------- | - |
| 1. | Operand type (singular) | [\<OperandType\>](#operandtype) | Information about a single operand type in the architecture. |

### \<OperandType\>
Hierarchy: \<Spec\> → \<ISA\> → \<OperandTypes\> → **\<OperandType\>**

Description: information about a single operand type in the architecture.

List of child elements:
| #  | Element                   | XML element name          | Description|
| -- | ------------------------- | ------------------------- | -|
| 1. | Operand type name         | \<OperandTypeName\>         | Name of the operand as defined in the ISA. For example, v1, s3, vcc, etc. |
| 2. | Subtypes                  | \<Subtypes\>                | Lists all subtype names that compose the give type. |
| 3. | Operand predefined values | [\<OperandPredefinedValues\>](#operandpredefinedvalues) | Lists all predefined operand values. A predefined value maps encoded integer value in the binary opcode to the corresponding assembly name. |

### \<OperandPredefinedValues\>
Hierarchy: \<Spec\> → \<ISA\> → \<OperandTypes\> →  \<OperandType\> → **\<OperandPredefinedValues\>**

Description: lists all predefined operand values. A predefined value maps encoded integer value in the binary opcode to the corresponding assembly name.

List of child elements:
| #  | Element                  | XML element name         | Description|
| -- | ------------------------ | ------------------------ | - |
| 1. | Operand predefined value | [\<OperandPredefinedValue\>](#operandpredefinedvalue) | Maps encoded integer value in the binary opcode to the corresponding assembly name. |


### \<OperandPredefinedValue\>
Hierarchy: \<Spec\> → \<ISA\> → \<OperandTypes\> →  \<OperandType\> → \<OperandPredefinedValues\>  → **\<OperandPredefinedValue\>**

Description: maps encoded integer value in the binary opcode to the corresponding assembly name.

List of child elements:
| #  | Element     | XML element name | Description |
| -- | ----------- | ---------------- | - |
| 1. | Name        | \<Name\>           | Assembly name of the operand. For example, v1, s3, vcc, etc. |
| 2. | Description | \<Description\>    | Description of the predefined value. |
| 3. | Value       | \<Value\>          | Integer value encoded in the binary opcode. This value is mapped to the assembly name.|

---

### \<FunctionalGroups\>
Hierarchy: \<Spec\> → \<ISA\> → **\<FunctionalGroups\>**

Description: lists all functional groups in the architecture. A function group provides high-level classification of the instructions, such as: vector memory, vector ALU, scalar memory, etc.

List of child elements:
| #  | Element          | XML element name | Description |
| -- | ---------------- | ---------------- | - |
| 1. | Functional Group (singular) | [\<FunctionalGroup\>](#functionalgroup) | Provides high-level classification of the instruction, such as: vector memory, vector ALU, scalar memory, etc. |

### \<FunctionalGroup\>
Hierarchy: \<Spec\> → \<ISA\> → \<Instructions\> → \<Instruction\> → \<FunctionalGroups\> → **\<FunctionalGroup\>**

Description: provides a high-level hardware functional group used for the instruction in this architecture.

List of child elements:
| #  | Element              | XML element name      | Description |
| -- | -------------------- | --------------------- | - |
| 1. | Name | \<Name\> | Name of the functional group. |
| 2. | Subgroup | [\<Subgroup\>](#Subgroup) | Name of the associated subgroup. |

### \<Subgroup\>
Hierarchy: \<Spec\> → \<ISA\> → \<Instructions\> → \<Instruction\> → \<FunctionalGroups\> → \<FunctionalGroup\> → **\<Subgroup\>**

Description: provides the associated subgroup for the instruction in this architecture. For example, a VMEM instruction can have the following subtypes: LOAD, STORE, ATOMIC, TEXTURE, etc.
