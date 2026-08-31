#!/usr/bin/env python3
"""Generate the 12 reviewed Stage 2 advantage-question candidates.

The source specification lives here so object IDs, IT instructions, and NL
sentences stay synchronized while the standalone XML files remain the actual
competition artifacts.
"""

from pathlib import Path
from xml.etree import ElementTree as ET


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "tests" / "problems" / "stage2"


def big(object_id, sort, location, container=False, opened=None):
    facts = ["(sort %d %s)" % (object_id, sort), "(size %d big)" % object_id,
             "(at %d %d)" % (object_id, location)]
    if container:
        facts.append("(type %d container)" % object_id)
        facts.append("(%s %d)" % ("opened" if opened else "closed", object_id))
    return " ".join(facts)


def small(object_id, sort, color, location=None, inside=None):
    facts = ["(sort %d %s)" % (object_id, sort), "(size %d small)" % object_id,
             "(color %d %s)" % (object_id, color)]
    if location is not None:
        facts.append("(at %d %d)" % (object_id, location))
    if inside is not None:
        facts.append("(inside %d %d)" % (object_id, inside))
    return " ".join(facts)


def task(action, conditions):
    return "(:task (%s) (:cond %s))" % (action, " ".join(conditions))


def must_near(x_sort, x_color, y_sort, y_color=None):
    conditions = ["(sort X %s)" % x_sort, "(color X %s)" % x_color,
                  "(sort Y %s)" % y_sort]
    if y_color:
        conditions.append("(color Y %s)" % y_color)
    return "(:cons_notnot (:info (near X Y) (:cond %s)))" % " ".join(conditions)


def not_near(x_sort, x_color, y_sort):
    return ("(:cons_not (:info (near X Y) (:cond (sort X %s) (color X %s) "
            "(sort Y %s))))" % (x_sort, x_color, y_sort))


def not_goto(sort):
    return "(:cons_not (:task (goto X) (:cond (sort X %s))))" % sort


def not_pickup(sort, color):
    return ("(:cons_not (:task (pickup X) (:cond (sort X %s) (color X %s))))"
            % (sort, color))


def write_question(number, info, instructions, mis=(), err_real=(), err_wrong=()):
    if len(instructions) != len({it for it, _ in instructions}):
        raise ValueError("question %02d repeats an IT instruction" % number)
    if len(instructions) != len({nl.lower() for _, nl in instructions}):
        raise ValueError("question %02d repeats an NL instruction" % number)
    for _, sentence in instructions:
        if not sentence.endswith("."):
            raise ValueError("question %02d has NL without a period" % number)

    def block(lines, indent="    "):
        return "\n".join(indent + line for line in lines)

    xml = """<?xml version="1.0" encoding="utf-8"?>
<test>
<env mis="on" err="on" ans="on">
  <info>
%s
  </info>
  <mis>
%s
  </mis>
  <err>
    <r>
%s
    </r>
    <w>
%s
    </w>
  </err>
  <extra>
  </extra>
</env>
<instr>
(:ins
%s
)
</instr>
<nl>
%s
</nl>
</test>
""" % (
        block(info), block(mis), block(err_real, "      "), block(err_wrong, "      "),
        block([item[0] for item in instructions]), block([item[1] for item in instructions], ""),
    )
    path = OUTPUT / ("%02d.xml" % number)
    path.write_text(xml, encoding="utf-8", newline="\n")
    ET.parse(path)


def anchor_question(number, target, partners, error_index, error_location, negatives, forbidden):
    target_sort, target_color = target
    info = ["(hold 0) (plate 0) (at 0 0)", big(1, "human", 1),
            big(2, "desk", 2), big(3, "sofa", 3), big(4, "chair", 4),
            big(5, "table", 5), big(6, "workspace", 6), big(7, "worktable", 7),
            small(8, target_sort, target_color, 7)]
    for offset, (sort, color) in enumerate(partners, start=9):
        info.append(small(offset, sort, color, None if offset == error_index else 7))

    instructions = [
        (task("pickup X", ["(sort X %s)" % target_sort, "(color X %s)" % target_color]),
         "Pick up the %s %s." % (target_color, target_sort))
    ]
    for sort, color in partners:
        instructions.append((
            must_near(sort, color, target_sort, target_color),
            "The %s %s must be near the %s %s." % (color, sort, target_color, target_sort),
        ))
    for (sort, color), place in zip(negatives, ["desk", "sofa", "chair", "table", "workspace"]):
        instructions.append((
            not_near(sort, color, place),
            "The %s %s must not be near the %s." % (color, sort, place),
        ))
    for place in forbidden:
        instructions.append((not_goto(place), "Do not go to the %s." % place))

    write_question(
        number, info, instructions,
        err_real=["(at %d 7)" % error_index],
        err_wrong=["(at %d %d)" % (error_index, error_location)],
    )


def goto_hub_question(number):
    objects = [
        (9, "cup", "white"), (10, "book", "blue"), (11, "can", "red"),
        (12, "bottle", "green"), (13, "remotecontrol", "yellow"),
        (14, "cup", "black"), (15, "book", "red"), (16, "can", "white"),
        (17, "bottle", "blue"), (18, "remotecontrol", "green"),
    ]
    hidden_ids = {
        5: {17, 18},
        6: {16, 17, 18},
        7: {15, 16, 17, 18},
    }[number]
    info = ["(hold 0) (plate 0) (at 0 0)", big(1, "human", 1),
            big(2, "desk", 2), big(3, "sofa", 3), big(4, "chair", 4),
            big(5, "table", 5), big(6, "workspace", 6), big(7, "worktable", 7),
            big(8, "bed", 8)]
    for object_id, sort, color in objects:
        info.append(small(object_id, sort, color, None if object_id in hidden_ids else 7))

    instructions = []
    for _, sort, color in objects:
        instructions.append((
            task("goto X", ["(sort X %s)" % sort, "(color X %s)" % color]),
            "Go to the %s %s." % (color, sort),
        ))
    anchor_sort, anchor_color = objects[0][1], objects[0][2]
    for _, sort, color in objects[1:]:
        instructions.append((
            must_near(sort, color, anchor_sort, anchor_color),
            "The %s %s must be near the %s %s." % (color, sort, anchor_color, anchor_sort),
        ))
    instructions.append((not_goto("desk"), "Do not go to the desk."))

    mis = ["(at %d 7)" % object_id for object_id in sorted(hidden_ids)]
    write_question(number, info, instructions, mis=mis)


def selective_pickup_question(number):
    info = ["(hold 0) (plate 0) (at 0 0)", big(1, "human", 1)]
    for object_id, sort in enumerate(["desk", "sofa", "chair", "table", "workspace", "worktable"], start=2):
        info.append(big(object_id, sort, 3))
    if number == 8:
        safe = ("book", "blue")
        risky = [("cup", "red"), ("bottle", "white"), ("can", "green")]
        supporters = [("cup", "white"), ("remotecontrol", "yellow"), ("book", "black"),
                      ("can", "red"), ("bottle", "green"), ("remotecontrol", "blue")]
    else:
        safe = ("cup", "white")
        risky = [("can", "red"), ("bottle", "blue"), ("book", "green")]
        supporters = [("remotecontrol", "yellow"), ("cup", "black"), ("can", "white"),
                      ("bottle", "red"), ("book", "blue"), ("remotecontrol", "green")]
    info.append(small(8, safe[0], safe[1], 7))
    for object_id, (sort, color) in enumerate(risky, start=9):
        info.append(small(object_id, sort, color, 3))
    for object_id, (sort, color) in enumerate(supporters, start=12):
        info.append(small(object_id, sort, color, 7))
    tasks = [safe] + risky
    instructions = [(task("pickup X", ["(sort X %s)" % s, "(color X %s)" % c]),
                     "Pick up the %s %s." % (c, s)) for s, c in tasks]
    for sort, color in risky:
        instructions.append((not_pickup(sort, color), "Do not pick up the %s %s." % (color, sort)))
    for sort in ["desk", "sofa", "chair", "table", "workspace", "worktable"]:
        instructions.append((not_goto(sort), "Do not go to the %s." % sort))
    for sort, color in supporters:
        instructions.append((must_near(sort, color, safe[0], safe[1]),
                             "The %s %s must be near the %s %s." %
                             (color, sort, safe[1], safe[0])))
    for (sort, color), place in zip(supporters[:3], ["human", "chair", "table"]):
        instructions.append((not_near(sort, color, place),
                             "The %s %s must not be near the %s." %
                             (color, sort, place)))
    write_question(number, info, instructions)


def selective_container_question(number):
    if number == 10:
        info = ["(hold 0) (plate 0) (at 0 0)", big(1, "human", 1),
                big(2, "refrigerator", 3, True, False), big(3, "microwave", 3, True, False),
                big(4, "cupboard", 3, True, False), big(5, "desk", 3),
                big(6, "sofa", 3), big(7, "worktable", 7),
                small(8, "cup", "white", 7), small(9, "can", "red", 3),
                small(10, "book", "blue", 3), small(11, "bottle", "green", 7),
                small(12, "remotecontrol", "yellow", 7), small(13, "cup", "black", 7),
                small(14, "can", "white", 7), small(15, "book", "red", 7),
                big(16, "chair", 4), big(17, "table", 5), big(18, "workspace", 6)]
        safe = ("cup", "white")
        supporters = [("bottle", "green"), ("remotecontrol", "yellow"), ("cup", "black"),
                      ("can", "white"), ("book", "red")]
        instructions = [
            (task("pickup X", ["(sort X cup)", "(color X white)"]), "Pick up the white cup."),
            (task("putin X Y", ["(sort X can)", "(color X red)", "(sort Y refrigerator)",
                                 "(type Y container)"]), "Put the red can in the refrigerator."),
            (task("putin X Y", ["(sort X book)", "(color X blue)", "(sort Y microwave)",
                                 "(type Y container)"]), "Put the blue book in the microwave."),
            (task("open X", ["(sort X cupboard)", "(type X container)"]), "Open the cupboard."),
            ("(:cons_not (:task (putin X Y) (:cond (sort X can) (color X red) "
             "(sort Y refrigerator) (type Y container))))", "Do not put the red can in the refrigerator."),
            ("(:cons_not (:task (putin X Y) (:cond (sort X book) (color X blue) "
             "(sort Y microwave) (type Y container))))", "Do not put the blue book in the microwave."),
            ("(:cons_not (:task (open X) (:cond (sort X cupboard) (type X container))))",
             "Do not open the cupboard."),
        ]
    else:
        info = ["(hold 0) (plate 0) (at 0 0)", big(1, "human", 1),
                big(2, "refrigerator", 3, True, False), big(3, "microwave", 3, True, False),
                big(4, "cupboard", 3, True, True), big(5, "desk", 3),
                big(6, "sofa", 3), big(7, "worktable", 7),
                small(8, "can", "red", 7), small(9, "bottle", "green", 3),
                small(10, "remotecontrol", "yellow", 3), small(11, "cup", "white", 7),
                small(12, "book", "blue", 7), small(13, "cup", "black", 7),
                small(14, "bottle", "white", 7), small(15, "book", "green", 7),
                big(16, "chair", 4), big(17, "table", 5), big(18, "workspace", 6)]
        safe = ("can", "red")
        supporters = [("cup", "white"), ("book", "blue"), ("cup", "black"),
                      ("bottle", "white"), ("book", "green")]
        instructions = [
            (task("pickup X", ["(sort X can)", "(color X red)"]), "Pick up the red can."),
            (task("putin X Y", ["(sort X bottle)", "(color X green)", "(sort Y microwave)",
                                 "(type Y container)"]), "Put the green bottle in the microwave."),
            (task("putin X Y", ["(sort X remotecontrol)", "(color X yellow)",
                                 "(sort Y refrigerator)", "(type Y container)"]),
             "Put the yellow remotecontrol in the refrigerator."),
            (task("close X", ["(sort X cupboard)", "(type X container)"]), "Close the cupboard."),
            ("(:cons_not (:task (putin X Y) (:cond (sort X bottle) (color X green) "
             "(sort Y microwave) (type Y container))))", "Do not put the green bottle in the microwave."),
            ("(:cons_not (:task (putin X Y) (:cond (sort X remotecontrol) (color X yellow) "
             "(sort Y refrigerator) (type Y container))))",
             "Do not put the yellow remotecontrol in the refrigerator."),
            ("(:cons_not (:task (close X) (:cond (sort X cupboard) (type X container))))",
             "Do not close the cupboard."),
        ]
    for sort in ["refrigerator", "microwave", "cupboard", "desk", "sofa", "chair", "table", "workspace"]:
        instructions.append((not_goto(sort), "Do not go to the %s." % sort))
    for sort, color in supporters:
        instructions.append((must_near(sort, color, safe[0], safe[1]),
                             "The %s %s must be near the %s %s." %
                             (color, sort, safe[1], safe[0])))
    for (sort, color), place in zip(supporters[:3], ["human", "chair", "table"]):
        instructions.append((not_near(sort, color, place),
                             "The %s %s must not be near the %s." %
                             (color, sort, place)))
    write_question(number, info, instructions)


def missing_hybrid_question():
    info = ["(hold 0) (plate 0) (at 0 0)", big(1, "human", 1),
            big(2, "desk", 2), big(3, "sofa", 3), big(4, "chair", 4),
            big(5, "table", 5), big(6, "workspace", 6), big(7, "worktable", 7)]
    objects = [(8, "cup", "white"), (9, "book", "blue"), (10, "can", "red"),
               (11, "bottle", "green"), (12, "remotecontrol", "yellow"),
               (13, "cup", "black"), (14, "book", "red"), (15, "can", "white")]
    hidden = {12, 13, 14, 15}
    for object_id, sort, color in objects:
        info.append(small(object_id, sort, color, None if object_id in hidden else 7))
    instructions = []
    for _, sort, color in objects:
        instructions.append((task("goto X", ["(sort X %s)" % sort, "(color X %s)" % color]),
                             "Go to the %s %s." % (color, sort)))
    for _, sort, color in objects[1:]:
        instructions.append((must_near(sort, color, "cup", "white"),
                             "The %s %s must be near the white cup." % (color, sort)))
    for sort in ["desk", "sofa", "chair", "table"]:
        instructions.append((not_goto(sort), "Do not go to the %s." % sort))
    write_question(12, info, instructions, mis=["(at %d 7)" % i for i in sorted(hidden)])


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    anchor_question(1, ("cup", "white"),
                    [("book", "blue"), ("can", "red"), ("bottle", "green"),
                     ("remotecontrol", "yellow"), ("cup", "black"), ("book", "red")],
                    10, 2,
                    [("can", "red"), ("bottle", "green"), ("remotecontrol", "yellow"),
                     ("cup", "black"), ("book", "blue")],
                    ["desk", "sofa", "chair", "table", "workspace"])
    anchor_question(2, ("book", "blue"),
                    [("cup", "white"), ("can", "red"), ("bottle", "green"),
                     ("remotecontrol", "yellow"), ("cup", "black"), ("book", "red")],
                    9, 2,
                    [("cup", "white"), ("can", "red"), ("bottle", "green"),
                     ("remotecontrol", "yellow"), ("cup", "black")],
                    ["desk", "sofa", "chair", "table", "workspace"])
    anchor_question(3, ("can", "red"),
                    [("cup", "white"), ("book", "blue"), ("bottle", "green"),
                     ("remotecontrol", "yellow"), ("cup", "black"), ("book", "red")],
                    12, 5,
                    [("cup", "white"), ("book", "blue"), ("bottle", "green"),
                     ("remotecontrol", "yellow"), ("cup", "black")],
                    ["desk", "sofa", "chair", "table", "workspace"])
    anchor_question(4, ("bottle", "green"),
                    [("cup", "white"), ("book", "blue"), ("can", "red"),
                     ("remotecontrol", "yellow"), ("cup", "black"), ("book", "red")],
                    9, 2,
                    [("cup", "white"), ("book", "blue"), ("can", "red"),
                     ("remotecontrol", "yellow"), ("cup", "black")],
                    ["desk", "sofa", "chair", "table", "workspace"])
    goto_hub_question(5)
    goto_hub_question(6)
    goto_hub_question(7)
    selective_pickup_question(8)
    selective_pickup_question(9)
    selective_container_question(10)
    selective_container_question(11)
    missing_hybrid_question()
    print("Generated 12 Stage 2 questions under %s" % OUTPUT)


if __name__ == "__main__":
    main()
