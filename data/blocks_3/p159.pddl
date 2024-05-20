;; blocks=2, percentage_new_tower=10, out_folder=., instance_id=159, seed=9

(define (problem blocksworld-159)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 - object)
 (:init
    (clear b2)
    (on b2 b1)
    (on-table b1))
 (:goal  (and
    (clear b2)
    (on b2 b1)
    (on-table b1))))
