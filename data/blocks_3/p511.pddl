;; blocks=5, percentage_new_tower=10, out_folder=., instance_id=511, seed=1

(define (problem blocksworld-511)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b5)
    (on b5 b4)
    (on b4 b3)
    (on-table b3)
    (clear b1)
    (on b1 b2)
    (on-table b2))
 (:goal  (and
    (clear b1)
    (on b1 b5)
    (on b5 b4)
    (on b4 b3)
    (on b3 b2)
    (on-table b2))))
