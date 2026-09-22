;; Joan Espasa Arxer (jea20@st-andrews.ac.uk), based on the expedition domain by Ben Pathak (pathak.ban@gmail.com)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Expedition domain
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Sleds need to follow a sequence of waypoints
;;; Sleds need supplies to move, and can retrieve and store supplies in waypoints
;;; Their objective is usually to arrive to the last waypoint
;;; There is possible interaction between sleds to cooperate.
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (domain expedition)
     (:requirements :typing :numeric-fluents)
     (:types
          sled waypoint - object
     )

     (:predicates
          (at ?s - sled ?w - waypoint)
          (is_next ?x - waypoint ?y - waypoint)
     )

     (:functions
          (sled_supplies ?s - sled)
          (sled_capacity ?s - sled)
          (waypoint_supplies ?w - waypoint)
     )
     ;; Move sled to the next waypoint
     (:action move_forwards
          :parameters (?s - sled ?w1 ?w2 - waypoint)
          :precondition (and (at ?s ?w1)
               (>= (sled_supplies ?s) 1)
               (is_next ?w1 ?w2))
          :effect (and (not (at ?s ?w1))
               (at ?s ?w2)
               (decrease (sled_supplies ?s) 1))
     )

     ;; Move sled to the previous waypoint
     (:action move_backwards
          :parameters (?s - sled ?w1 ?w2 - waypoint)
          :precondition (and (at ?s ?w1)
               (>= (sled_supplies ?s) 1)
               (is_next ?w2 ?w1))
          :effect (and (not (at ?s ?w1))
               (at ?s ?w2)
               (decrease (sled_supplies ?s) 1))
     )

     ;; Store sled supplies
     (:action store_supplies
          :parameters (?s - sled ?w - waypoint)
          :precondition (and (at ?s ?w)
               (>= (sled_supplies ?s) 1))
          :effect (and (increase (waypoint_supplies ?w) 1)
               (decrease (sled_supplies ?s) 1))
     )

     ;; Retrieve waypoint supplies
     (:action retrieve_supplies
          :parameters (?s - sled ?w - waypoint)
          :precondition (and (at ?s ?w)
               (>= (waypoint_supplies ?w) 1)
               (> (sled_capacity ?s) (sled_supplies ?s)))
          :effect (and (decrease (waypoint_supplies ?w) 1)
               (increase (sled_supplies ?s) 1))
     )
)
