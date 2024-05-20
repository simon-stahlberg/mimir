;; blocks=2, percentage_new_tower=0, out_folder=., instance_id=147, seed=27

(define (problem blocksworld-147)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 - object)
 (:init
    (clear b1)
    (on b1 b2)
    (on-table b2))
 (:goal  (and
    (clear b1)
    (on b1 b2)
    (on-table b2))))
