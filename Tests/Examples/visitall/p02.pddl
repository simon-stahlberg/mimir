(define (problem visitall-admissible-chain-test)
(:domain grid-visit-all)
(:objects
	loc-0
	loc-1
	loc-2
	loc-3
- place

)
(:init
	(at-robot loc-0)
	(visited loc-0)
	(connected loc-0 loc-1)
 	(connected loc-1 loc-0)
	(connected loc-0 loc-2)
 	(connected loc-2 loc-0)
 	(connected loc-1 loc-3)
 	(connected loc-3 loc-1)
	(connected loc-2 loc-3)
 	(connected loc-3 loc-2)

)
(:goal
(and
	(visited loc-0)
	(visited loc-1)
	(visited loc-2)
	(visited loc-3)
)
)
)
