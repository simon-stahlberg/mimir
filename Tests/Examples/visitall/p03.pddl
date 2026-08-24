(define (problem grid-2)
(:domain grid-visit-all)
(:objects
	loc-0
	loc-1
	loc-2
- place

)
(:init
	(at-robot loc-0)
	(visited loc-0)
	(connected loc-0 loc-1)
 	(connected loc-1 loc-0)
 	(connected loc-1 loc-2)
 	(connected loc-2 loc-1)

)
(:goal
(and
	(visited loc-0)
	(visited loc-1)
	(visited loc-2)
)
)
)
