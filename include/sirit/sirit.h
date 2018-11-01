/* This file is part of the sirit project.
 * Copyright (c) 2018 ReinUsesLisp
 * This software may be used and distributed according to the terms of the GNU
 * Lesser General Public License version 2.1 or any later version.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <set>
#include <spirv/unified1/spirv.hpp11>
#include <variant>
#include <vector>

namespace Sirit {

constexpr std::uint32_t GENERATOR_MAGIC_NUMBER = 0;

class Op;
class Operand;

using Literal = std::variant<std::uint32_t, std::uint64_t, std::int32_t,
                             std::int64_t, float, double>;
using Id = const Op*;

class Module {
  public:
    explicit Module();
    ~Module();

    /**
     * Assembles current module into a SPIR-V stream.
     * It can be called multiple times but it's recommended to copy code
     * externally.
     * @return A stream of bytes representing a SPIR-V module.
     */
    std::vector<std::uint8_t> Assemble() const;

    /// Adds a module capability.
    void AddCapability(spv::Capability capability);

    /// Sets module memory model.
    void SetMemoryModel(spv::AddressingModel addressing_model,
                        spv::MemoryModel memory_model);

    /// Adds an entry point.
    void AddEntryPoint(spv::ExecutionModel execution_model, Id entry_point,
                       const std::string& name,
                       const std::vector<Id>& interfaces = {});

    /**
     * Adds an instruction to module's code
     * @param op Instruction to insert into code. Types and constants must not
     * be emitted.
     * @return Returns op.
     */
    Id Emit(Id op);

    /**
     * Adds a global variable
     * @param variable Global variable to add.
     * @return Returns variable.
     */
    Id AddGlobalVariable(Id variable);

    // Types

    /// Returns type void.
    Id OpTypeVoid();

    /// Returns type bool.
    Id OpTypeBool();

    /// Returns type integer.
    Id OpTypeInt(int width, bool is_signed);

    /// Returns type float.
    Id OpTypeFloat(int width);

    /// Returns type vector.
    Id OpTypeVector(Id component_type, int component_count);

    /// Returns type matrix.
    Id OpTypeMatrix(Id column_type, int column_count);

    /// Returns type image.
    Id OpTypeImage(Id sampled_type, spv::Dim dim, int depth, bool arrayed,
                   bool ms, int sampled, spv::ImageFormat image_format,
                   std::optional<spv::AccessQualifier> access_qualifier = {});

    /// Returns type sampler.
    Id OpTypeSampler();

    /// Returns type sampled image.
    Id OpTypeSampledImage(Id image_type);

    /// Returns type array.
    Id OpTypeArray(Id element_type, Id length);

    /// Returns type runtime array.
    Id OpTypeRuntimeArray(Id element_type);

    /// Returns type struct.
    Id OpTypeStruct(const std::vector<Id>& members = {});

    /// Returns type opaque.
    Id OpTypeOpaque(const std::string& name);

    /// Returns type pointer.
    Id OpTypePointer(spv::StorageClass storage_class, Id type);

    /// Returns type function.
    Id OpTypeFunction(Id return_type, const std::vector<Id>& arguments = {});

    /// Returns type event.
    Id OpTypeEvent();

    /// Returns type device event.
    Id OpTypeDeviceEvent();

    /// Returns type reserve id.
    Id OpTypeReserveId();

    /// Returns type queue.
    Id OpTypeQueue();

    /// Returns type pipe.
    Id OpTypePipe(spv::AccessQualifier access_qualifier);

    // Constant

    /// Returns a true scalar constant.
    Id OpConstantTrue(Id result_type);

    /// Returns a false scalar constant.
    Id OpConstantFalse(Id result_type);

    /// Returns a numeric scalar constant.
    Id OpConstant(Id result_type, const Literal& literal);

    /// Returns a numeric scalar constant.
    Id OpConstantComposite(Id result_type, const std::vector<Id>& constituents);

    /// Returns a sampler constant.
    Id OpConstantSampler(Id result_type,
                         spv::SamplerAddressingMode addressing_mode,
                         bool normalized, spv::SamplerFilterMode filter_mode);

    /// Returns a null constant value.
    Id OpConstantNull(Id result_type);

    // Function

    /// Declares a function.
    Id OpFunction(Id result_type, spv::FunctionControlMask function_control,
                  Id function_type);

    /// Ends a function.
    Id OpFunctionEnd();

    /// Call a function.
    Id OpFunctionCall(Id result_type, Id function,
                      const std::vector<Id>& arguments = {});

    // Flow

    /// Declare a structured loop.
    Id OpLoopMerge(Id merge_block, Id continue_target,
                   spv::LoopControlMask loop_control,
                   const std::vector<Id>& literals = {});

    /// Declare a structured selection.
    Id OpSelectionMerge(Id merge_block,
                        spv::SelectionControlMask selection_control);

    /// The block label instruction: Any reference to a block is through this
    /// ref.
    Id OpLabel();

    /// Unconditional jump to label.
    Id OpBranch(Id target_label);

    /// If condition is true branch to true_label, otherwise branch to
    /// false_label.
    Id OpBranchConditional(Id condition, Id true_label, Id false_label,
                           std::uint32_t true_weight = 0,
                           std::uint32_t false_weight = 0);

    /// Returns with no value from a function with void return type.
    Id OpReturn();

    /// Return a value from a function.
    Id OpReturnValue(Id value);

    // Debug

    /// Assign a name string to a reference.
    /// @return target
    Id Name(Id target, const std::string& name);

    // Memory

    /// Allocate an object in memory, resulting in a copy to it.
    Id OpVariable(Id result_type, spv::StorageClass storage_class,
                  Id initializer = nullptr);

    /// Load through a pointer.
    Id OpLoad(Id result_type, Id pointer,
              std::optional<spv::MemoryAccessMask> memory_access = {});

    /// Store through a pointer.
    Id OpStore(Id pointer, Id object,
               std::optional<spv::MemoryAccessMask> memory_access = {});

    /// Create a pointer into a composite object that can be used with OpLoad
    /// and OpStore.
    Id OpAccessChain(Id result_type, Id base,
                     const std::vector<Id>& indexes = {});

    /// Make a copy of a composite object, while modifying one part of it.
    Id OpCompositeInsert(Id result_type, Id object, Id composite,
                         const std::vector<Literal>& indexes = {});

    // Annotation

    /// Add a decoration to target.
    Id Decorate(Id target, spv::Decoration decoration,
                const std::vector<Literal>& literals = {});

    Id MemberDecorate(Id structure_type, Literal member,
                      spv::Decoration decoration,
                      const std::vector<Literal>& literals = {});

    // Misc

    /// Make an intermediate object whose value is undefined.
    Id OpUndef(Id result_type);

  private:
    Id AddCode(std::unique_ptr<Op> op);

    Id AddCode(spv::Op opcode, std::optional<std::uint32_t> id = {});

    Id AddDeclaration(std::unique_ptr<Op> op);

    Id AddAnnotation(std::unique_ptr<Op> op);

    std::uint32_t bound{1};

    std::set<spv::Capability> capabilities;

    std::set<std::string> extensions;

    std::set<std::unique_ptr<Op>> ext_inst_import;

    spv::AddressingModel addressing_model{spv::AddressingModel::Logical};
    spv::MemoryModel memory_model{spv::MemoryModel::GLSL450};

    std::vector<std::unique_ptr<Op>> entry_points;

    std::vector<std::unique_ptr<Op>> execution_mode;

    std::vector<std::unique_ptr<Op>> debug;

    std::vector<std::unique_ptr<Op>> annotations;

    std::vector<std::unique_ptr<Op>> declarations;

    std::vector<Id> global_variables;

    std::vector<Id> code;

    std::vector<std::unique_ptr<Op>> code_store;
};

} // namespace Sirit
