;; The Hydro Power domain is is concerned with generating financial gains,
;; obtained by pumping up water when energy is cheap, storing it using hydroelectric
;; reservoirs and selling it when the price is higher. Introduced in

;; Coles, Amanda, M. Fox, and D. Long.
;; "A hybrid LP-RPG heuristic for modelling numeric resource flows in planning."
;; Journal of Artificial Intelligence Research 46 (2013): 343-412.
(define (domain hydropower)

	(:requirements :adl :typing :numeric-fluents)

	(:types
		powerstation time turnvalue
	)

	(:predicates

		(fewerturns ?n1 - turnvalue ?n2 - turnvalue)
		(timenow ?t - time)
		(before ?t1 - time ?t2 - time)
		(demand ?t - time ?n - turnvalue)

		;		(faulthappened ?t - time ?c - circuit)
		;		(needfault ?c - circuit ?t - time)

		;		(sundonfaulthappened ?t - time)
		;		(needsundonfault ?t - time)

		(faultrepair ?t - time)
		(faultrepaired ?t - time)

		;		(okayfault ?c - circuit ?t - time)

		(transitive-later-than ?t1 - time ?t2 - time)
	)

	(:functions
		(funds)
		(stored_units)
		(stored_capacity)
		(value ?n - turnvalue)

	)

	;;;;;;;;;;;;advancing time

	(:action advance_time
		:parameters(?t1 - time ?t2 - time)
		:precondition (and
			(timenow ?t1)
			(before ?t1 ?t2)
		)
		:effect( and
			(timenow ?t2)
			(not (timenow ?t1))
		)
	)

	(:action pump_water_up
		:parameters(?t1 - time ?n1 - turnvalue)
		:precondition (and
			(timenow ?t1)
			(>= (funds) (* 1.05 (value ?n1)))
			(>= (stored_capacity) 1)
			(demand ?t1 ?n1)
		)
		:effect( and
			(increase (stored_units) 1)
			(decrease (stored_capacity) 1)
			(decrease (funds) (* 1.05 (value ?n1)))
		)
	)

	(:action generate
		:parameters(?t1 - time ?n1 - turnvalue)
		:precondition (and
			(timenow ?t1)
			(>= (stored_units) 1)
			(demand ?t1 ?n1)
		)
		:effect( and
			(decrease (stored_units) 1)
			(increase (stored_capacity) 1)
			(increase (funds) (value ?n1))
		)
	)

	;;;;;;;;;;;;induction of faults
	;	(:action induce_fault
	;		:parameters (?t - time ?c - circuit)
	;		:precondition (and
	;				(timenow ?t)
	;				(needfault ?c ?t)
	;			)
	;		:effect(and
	;			(faulthappened ?t ?c)
	;			(not (needfault ?c ?t))
	;			(decrease (voltage ?c) 2)
	;			(forall (?tf - time) (when (transitive-later-than ?tf ?t) (decrease (voltage_was ?c ?tf) 2)))
	;
	;		)
	;	)
	;
	;	(:action repair_fault
	;		:parameters (?t - time ?c - circuit)
	;		:precondition (and
	;				(timenow ?t)
	;				(faultrepair ?t ?c)
	;			)
	;		:effect(and
	;			(faultrepaired ?t ?c)
	;			(increase (voltage ?c) 2)
	;			(forall (?tf - time) (when (transitive-later-than ?tf ?t) (increase (voltage_was ?c ?tf) 2)))
	;			(not (faultrepair ?t ?c))
	;		)
	;	)
	;
	;
	;	(:action sundon_line_loss
	;		:parameters (?t - time ?n - turnvalue ?c - circuit)
	;		:precondition (and
	;				(timenow ?t)
	;				(needsundonfault ?c ?t)
	;			)
	;		:effect(and
	;			(sundonfaulthappened ?t ?c)
	;			(increase (voltage ?c) 1)
	;			(forall (?tf - time) (when (transitive-later-than ?tf ?t) (increase (voltage_was ?c ?tf) 1)))
	;		)
	;	)

)
