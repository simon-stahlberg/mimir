#! /usr/bin/env python

from lab.parser import Parser
from lab import tools
import re


def aggregate_generator_statistics(content, props):
    if "num_generated_valid_action_base_bindings" in props or "num_generated_valid_axiom_base_bindings" in props:
        props["num_generated_valid_base_bindings"] = props.get("num_generated_valid_action_base_bindings", 0) + props.get("num_generated_valid_axiom_base_bindings", 0)
    if "num_generated_invalid_action_base_bindings" in props or "num_generated_invalid_axiom_base_bindings" in props:
        props["num_generated_invalid_base_bindings"] = props.get("num_generated_invalid_action_base_bindings", 0) + props.get("num_generated_invalid_axiom_base_bindings", 0)   
    if "num_generated_valid_action_derived_bindings" in props or "num_generated_valid_axiom_derived_bindings" in props:
        props["num_generated_valid_derived_bindings"] = props.get("num_generated_valid_action_derived_bindings", 0) + props.get("num_generated_valid_axiom_derived_bindings", 0)
    if "num_generated_invalid_action_derived_bindings" in props or "num_generated_invalid_axiom_derived_bindings" in props:
        props["num_generated_invalid_derived_bindings"] = props.get("num_generated_invalid_action_derived_bindings", 0) + props.get("num_generated_invalid_axiom_derived_bindings", 0)   
    
    if "num_generated_valid_action_base_bindings_until_last_f_layer" in props or "num_generated_valid_axiom_base_bindings_until_last_f_layer" in props:
        props["num_generated_valid_base_bindings_until_last_f_layer"] = props.get("num_generated_valid_action_base_bindings_until_last_f_layer", 0) + props.get("num_generated_valid_axiom_base_bindings_until_last_f_layer", 0)
    if "num_generated_invalid_action_base_bindings_until_last_f_layer" in props or "num_generated_invalid_axiom_base_bindings_until_last_f_layer" in props:
        props["num_generated_invalid_base_bindings_until_last_f_layer"] = props.get("num_generated_invalid_action_base_bindings_until_last_f_layer", 0) + props.get("num_generated_invalid_axiom_base_bindings_until_last_f_layer", 0)   
    if "num_generated_valid_action_derived_bindings_until_last_f_layer" in props or "num_generated_valid_axiom_derived_bindings_until_last_f_layer" in props:
        props["num_generated_valid_derived_bindings_until_last_f_layer"] = props.get("num_generated_valid_action_derived_bindings_until_last_f_layer", 0) + props.get("num_generated_valid_axiom_derived_bindings_until_last_f_layer", 0)
    if "num_generated_invalid_action_derived_bindings_until_last_f_layer" in props or "num_generated_invalid_axiom_derived_bindings_until_last_f_layer" in props:
        props["num_generated_invalid_derived_bindings_until_last_f_layer"] = props.get("num_generated_invalid_action_derived_bindings_until_last_f_layer", 0) + props.get("num_generated_invalid_axiom_derived_bindings_until_last_f_layer", 0)  


def add_overapproximation_ratios(content, props):
    if "num_generated_valid_base_bindings" in props and "num_generated_invalid_base_bindings"  in props:
        if props["num_generated_valid_base_bindings"] != 0:
            props["overapproximation_ratio"] = (props["num_generated_invalid_base_bindings"] + props["num_generated_valid_base_bindings"]) / props["num_generated_valid_base_bindings"]
    if "num_generated_valid_base_bindings_until_last_f_layer" in props and "num_generated_invalid_base_bindings_until_last_f_layer" in props:
        if props["num_generated_valid_base_bindings_until_last_f_layer"] != 0:
            props["overapproximation_ratio_until_last_f_layer"] = (props["num_generated_invalid_base_bindings_until_last_f_layer"] + props["num_generated_valid_base_bindings_until_last_f_layer"]) / props["num_generated_valid_base_bindings_until_last_f_layer"]



def combine_schema_statistics(content, props):
    for i in range(5):
        props[f"num_schemas_by_arity_{i}"] = props.get(f"num_actions_by_arity_{i}", 0) + props.get(f"num_axioms_by_arity_{i}", 0)
    props["num_schemas_by_arity_greater_or_equal_5"] = props.get("num_actions_by_arity_greater_or_equal_5", 0) + props.get("num_axioms_by_arity_greater_or_equal_5", 0)


def translate_total_time_to_sec(content, props):
    if "total_time" in props:
        props["total_time"] = int((props["total_time"]+999) / 1000)



class GeneratorParser(Parser):
    """
    Num predicates by arity: [0, 8, 1]
    Num functions by arity: [0, 2, 3]
    Num constraints by arity: [0, 0, 4]
    [LiftedApplicableActionGenerator] Number of grounded action cache hits: 34510
    [LiftedApplicableActionGenerator] Number of grounded action cache hits until last f-layer: 11219
    [LiftedApplicableActionGenerator] Number of grounded action cache misses: 56
    [LiftedApplicableActionGenerator] Number of grounded action cache misses until last f-layer: 48
    [SatisficingBindingGenerator] Number of generated valid base bindings: 42185
    [SatisficingBindingGenerator] Number of generated valid base bindings until last f-layer: 13786
    [SatisficingBindingGenerator] Number of generated valid derived bindings: 34566
    [SatisficingBindingGenerator] Number of generated valid derived bindings until last f-layer: 11267
    [SatisficingBindingGenerator] Number of generated invalid base bindings: 0
    [SatisficingBindingGenerator] Number of generated invalid base bindings until last f-layer: 0
    [SatisficingBindingGenerator] Number of generated invalid derived bindings: 7619
    [SatisficingBindingGenerator] Number of generated invalid derived bindings until last f-layer: 2519
    [LiftedAxiomEvaluator] Number of grounded axiom cache hits: 0
    [LiftedAxiomEvaluator] Number of grounded axiom cache hits until last f-layer: 0
    [LiftedAxiomEvaluator] Number of grounded axiom cache misses: 0
    [LiftedAxiomEvaluator] Number of grounded axiom cache misses until last f-layer: 0
    [SatisficingBindingGenerator] Number of generated valid base bindings: 0
    [SatisficingBindingGenerator] Number of generated valid base bindings until last f-layer: 0
    [SatisficingBindingGenerator] Number of generated valid derived bindings: 0
    [SatisficingBindingGenerator] Number of generated valid derived bindings until last f-layer: 0
    [SatisficingBindingGenerator] Number of generated invalid base bindings: 0
    [SatisficingBindingGenerator] Number of generated invalid base bindings until last f-layer: 0
    [SatisficingBindingGenerator] Number of generated invalid derived bindings: 0
    [SatisficingBindingGenerator] Number of generated invalid derived bindings until last f-layer: 0
    """
    def __init__(self):
        super().__init__()

        self.add_pattern(
            "num_generated_valid_action_base_bindings", 
            r"\[LiftedApplicableActionGenerator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated valid base bindings: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_invalid_action_base_bindings", 
            r"\[LiftedApplicableActionGenerator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated invalid base bindings: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_valid_action_derived_bindings", 
            r"\[LiftedApplicableActionGenerator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated valid derived bindings: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_invalid_action_derived_bindings", 
            r"\[LiftedApplicableActionGenerator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated invalid derived bindings: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_valid_axiom_base_bindings", 
            r"\[LiftedAxiomEvaluator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated valid base bindings: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_invalid_axiom_base_bindings", 
            r"\[LiftedAxiomEvaluator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated invalid base bindings: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_valid_axiom_derived_bindings", 
            r"\[LiftedAxiomEvaluator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated valid derived bindings: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_invalid_axiom_derived_bindings", 
            r"\[LiftedAxiomEvaluator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated invalid derived bindings: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_valid_action_base_bindings_until_last_f_layer", 
            r"\[LiftedApplicableActionGenerator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated valid base bindings until last f-layer: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_invalid_action_base_bindings_until_last_f_layer", 
            r"\[LiftedApplicableActionGenerator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated invalid base bindings until last f-layer: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_valid_action_derived_bindings_until_last_f_layer", 
            r"\[LiftedApplicableActionGenerator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated valid derived bindings until last f-layer: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_invalid_action_derived_bindings_until_last_f_layer", 
            r"\[LiftedApplicableActionGenerator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated invalid derived bindings until last f-layer: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_valid_axiom_base_bindings_until_last_f_layer", 
            r"\[LiftedAxiomEvaluator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated valid base bindings until last f-layer: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_invalid_axiom_base_bindings_until_last_f_layer", 
            r"\[LiftedAxiomEvaluator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated invalid base bindings until last f-layer: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_valid_axiom_derived_bindings_until_last_f_layer", 
            r"\[LiftedAxiomEvaluator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated valid derived bindings until last f-layer: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_invalid_axiom_derived_bindings_until_last_f_layer", 
            r"\[LiftedAxiomEvaluator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated invalid derived bindings until last f-layer: (\d+)",
            type=int,
            flags="S")
        
        self.add_function(aggregate_generator_statistics)
        self.add_function(add_overapproximation_ratios)
 
        for i in range(5):
            predicates_pattern = r"Num predicates by arity: \["
            functions_pattern = r"Num functions by arity: \["
            constraints_pattern = r"Num constraints by arity: \["
            actions_pattern = r"Num actions by arity: \["
            axioms_pattern = r"Num axioms by arity: \["
            for _ in range(i):
                predicates_pattern += r"\d+, "
                functions_pattern += r"\d+, "
                constraints_pattern += r"\d+, "
                actions_pattern += r"\d+, "
                axioms_pattern += r"\d+, "
            predicates_pattern += r"(\d+)"
            functions_pattern += r"(\d+)"
            constraints_pattern += r"(\d+)"
            actions_pattern += r"(\d+)"
            axioms_pattern += r"(\d+)"

            self.add_pattern(
                f"num_predicates_by_arity_{i}", predicates_pattern, type=int)
            self.add_pattern(
                f"num_functions_by_arity_{i}", functions_pattern, type=int)
            self.add_pattern(
                f"num_constraints_by_arity_{i}", constraints_pattern, type=int)
            self.add_pattern(
                f"num_actions_by_arity_{i}", actions_pattern, type=int)
            self.add_pattern(
                f"num_axioms_by_arity_{i}", axioms_pattern, type=int)
            
        self.add_pattern(
            f"num_predicates_by_arity_greater_or_equal_5", r"Num predicates by oob arity: (\d+)", type=int)
        self.add_pattern(
            f"num_functions_by_arity_greater_or_equal_5", r"Num functions by oob arity: (\d+)", type=int)
        self.add_pattern(
            f"num_constraints_by_arity_greater_or_equal_5", r"Num constraints by oob arity: (\d+)", type=int)
        self.add_pattern(
            f"num_actions_by_arity_greater_or_equal_5", r"Num actions by oob arity: (\d+)", type=int)
        self.add_pattern(
            f"num_axioms_by_arity_greater_or_equal_5", r"Num axioms by oob arity: (\d+)", type=int)
        
        self.add_function(combine_schema_statistics)

        self.add_function(translate_total_time_to_sec)



    