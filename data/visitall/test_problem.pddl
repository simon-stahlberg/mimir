(define (problem grid-3)
(:domain grid-visit-all)
(:requirements :typing)
(:objects
	loc-x0-y0
	loc-x0-y1
	loc-x0-y2
	loc-x1-y0
	loc-x1-y2
	loc-x2-y0
	loc-x2-y1
- place

)
(:init
	(at-robot loc-x0-y1)
	(visited loc-x0-y1)
	(connected loc-x0-y0 loc-x1-y0)
 	(connected loc-x0-y0 loc-x0-y1)
 	(connected loc-x0-y1 loc-x0-y0)
 	(connected loc-x0-y1 loc-x0-y2)
 	(connected loc-x0-y2 loc-x1-y2)
 	(connected loc-x0-y2 loc-x0-y1)
 	(connected loc-x1-y0 loc-x0-y0)
 	(connected loc-x1-y0 loc-x2-y0)
 	(connected loc-x1-y2 loc-x0-y2)
 	(connected loc-x2-y0 loc-x1-y0)
 	(connected loc-x2-y0 loc-x2-y1)
 	(connected loc-x2-y1 loc-x2-y0)

)
(:goal
(and
	(visited loc-x0-y0)
	(visited loc-x0-y1)
	(visited loc-x0-y2)
	(visited loc-x1-y0)
	(visited loc-x1-y2)
	(visited loc-x2-y0)
	(visited loc-x2-y1)
)
)
)
