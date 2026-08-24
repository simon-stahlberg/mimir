; Transport weighted path choice: one short expensive route and one long cheap route

(define (problem transport-weighted-path-choice)
 (:domain transport)
 (:requirements :typing :action-costs)
 (:objects
  depot - location
  mid-1 - location
  mid-2 - location
  destination - location
  truck-1 - vehicle
  package-1 - package
  capacity-0 - capacity-number
  capacity-1 - capacity-number
 )
 (:init
  (= (total-cost) 0)
  (capacity-predecessor capacity-0 capacity-1)

  (road depot destination)
  (= (road-length depot destination) 100)

  (road depot mid-1)
  (= (road-length depot mid-1) 10)

  (road mid-1 mid-2)
  (= (road-length mid-1 mid-2) 10)

  (road mid-2 destination)
  (= (road-length mid-2 destination) 10)

  (at truck-1 depot)
  (capacity truck-1 capacity-1)
  (at package-1 depot)
 )
 (:goal (and
  (at package-1 destination)
 ))
 (:metric minimize (total-cost))
)
