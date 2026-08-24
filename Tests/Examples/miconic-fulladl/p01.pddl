(define (problem num_passengers_2)
   (:domain miconic)
   (:requirements :adl)
   (:objects
      p0 p1 p2 - passenger
      f0 f1 f2 - floor)

   (:init
      (conflict_B p0)
      (conflict_B p1)
      (above f0 f1)
      (above f0 f2)
      (origin p0 f0)
      (origin p1 f1)
      (origin p2 f2)
      (destin p0 f2)
      (destin p1 f1)
      (destin p2 f1)
      (lift-at f0))
   (:goal (forall (?p - passenger) (served ?p)))
)


