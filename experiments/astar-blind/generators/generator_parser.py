#! /usr/bin/env python

from lab.parser import Parser
from lab import tools
import re


def aggregate_generator_statistics(content, props):
    if "num_generated_valid_action_bindings" in props or "num_generated_valid_axiom_bindings" in props:
        props["num_generated_valid_bindings"] = props.get("num_generated_valid_action_bindings", 0) + props.get("num_generated_valid_axiom_bindings", 0)
    if "num_generated_invalid_action_bindings" in props or "num_generated_invalid_axiom_bindings" in props:
        props["num_generated_invalid_bindings"] = props.get("num_generated_invalid_action_bindings", 0) + props.get("num_generated_invalid_axiom_bindings", 0)   
    if "num_generated_valid_action_bindings_until_last_f_layer" in props or "num_generated_valid_axiom_bindings_until_last_f_layer" in props:
        props["num_generated_valid_bindings_until_last_f_layer"] = props.get("num_generated_valid_action_bindings_until_last_f_layer", 0) + props.get("num_generated_valid_axiom_bindings_until_last_f_layer", 0)
    if "num_generated_invalid_action_bindings_until_last_f_layer" in props or "num_generated_invalid_axiom_bindings_until_last_f_layer" in props:
        props["num_generated_invalid_bindings_until_last_f_layer"] = props.get("num_generated_invalid_action_bindings_until_last_f_layer", 0) + props.get("num_generated_invalid_axiom_bindings_until_last_f_layer", 0)   
    




class GeneratorParser(Parser):
    """
    [LiftedApplicableActionGenerator] Number of grounded action cache hits: 1905625
    [LiftedApplicableActionGenerator] Number of grounded action cache hits until last f-layer: 1902981
    [LiftedApplicableActionGenerator] Number of grounded action cache misses: 2
    [LiftedApplicableActionGenerator] Number of grounded action cache misses until last f-layer: 2
    [SatisficingBindingGenerator] Number of generated valid bindings: 1905627
    [SatisficingBindingGenerator] Number of generated valid bindings until last f-layer: 0
    [SatisficingBindingGenerator] Number of generated invalid bindings: 3357305241
    [SatisficingBindingGenerator] Number of generated invalid bindings until last f-layer: 0
    [LiftedAxiomEvaluator] Number of grounded axiom cache hits: 0
    [LiftedAxiomEvaluator] Number of grounded axiom cache hits until last f-layer: 0
    [LiftedAxiomEvaluator] Number of grounded axiom cache misses: 0
    [LiftedAxiomEvaluator] Number of grounded axiom cache misses until last f-layer: 0
    [SatisficingBindingGenerator] Number of generated valid bindings: 0
    [SatisficingBindingGenerator] Number of generated valid bindings until last f-layer: 0
    [SatisficingBindingGenerator] Number of generated invalid bindings: 0
    [SatisficingBindingGenerator] Number of generated invalid bindings until last f-layer: 0
    """
    def __init__(self):
        super().__init__()

        self.add_pattern(
            "num_generated_valid_action_bindings", 
            r"\[LiftedApplicableActionGenerator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated valid bindings: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_invalid_action_bindings", 
            r"\[LiftedApplicableActionGenerator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated invalid bindings: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_valid_axiom_bindings", 
            r"\[LiftedAxiomEvaluator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated valid bindings: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_invalid_axiom_bindings", 
            r"\[LiftedAxiomEvaluator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated invalid bindings: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_valid_action_bindings_until_last_f_layer", 
            r"\[LiftedApplicableActionGenerator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated valid bindings until last f-layer: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_invalid_action_bindings_until_last_f_layer", 
            r"\[LiftedApplicableActionGenerator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated invalid bindings until last f-layer: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_valid_axiom_bindings_until_last_f_layer", 
            r"\[LiftedAxiomEvaluator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated valid bindings until last f-layer: (\d+)",
            type=int,
            flags="S")
        
        self.add_pattern(
            "num_generated_invalid_axiom_bindings_until_last_f_layer", 
            r"\[LiftedAxiomEvaluator\]"
            r".*?"      
            r"\[SatisficingBindingGenerator\] Number of generated invalid bindings until last f-layer: (\d+)",
            type=int,
            flags="S")
        
        self.add_function(aggregate_generator_statistics)

        