;; blocks=5, percentage_new_tower=10, out_folder=., instance_id=525, seed=15

(define (problem blocksworld-525)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b4)
    (on b4 b2)
    (on b2 b5)
    (on b5 b3)
    (on-table b3)
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b4)
    (on b4 b2)
    (on-table b2)
    (clear b3)
    (on b3 b1)
    (on b1 b5)
    (on-table b5))))
