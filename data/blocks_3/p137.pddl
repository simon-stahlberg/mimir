;; blocks=2, percentage_new_tower=0, out_folder=., instance_id=137, seed=17

(define (problem blocksworld-137)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 - object)
 (:init
    (clear b2)
    (on b2 b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on b1 b2)
    (on-table b2))))
