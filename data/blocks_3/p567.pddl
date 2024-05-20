;; blocks=5, percentage_new_tower=20, out_folder=., instance_id=567, seed=27

(define (problem blocksworld-567)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b1)
    (on b1 b3)
    (on b3 b2)
    (on b2 b4)
    (on b4 b5)
    (on-table b5))
 (:goal  (and
    (clear b5)
    (on-table b5)
    (clear b1)
    (on b1 b4)
    (on b4 b2)
    (on b2 b3)
    (on-table b3))))
