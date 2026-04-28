"""Validate Day 1 placed password doors inside the Unreal Editor.

Run from UnrealEditor-Cmd with:
  -ExecutePythonScript=Scripts/Validation/VerifyDay1PasswordDoors.py
"""

import unreal


TARGET_MAPS = [
    "/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1",
]

EXPECTED_PASSWORD_DOORS = {
    "DeepWater_PasswordDoor_0417": "0417",
    "DeepWater_PasswordHatch_1939": "1939",
    "DeepWater_PasswordDoor_1831": "1831",
    "DeepWater_PasswordHatch_1799": "1799",
    "DeepWater_PasswordDoor_1697": "1697",
    # These are present in the map audit even though they are not part of the
    # primary route lock yet. Keep them validated so stray blockers do not ship.
    "DeepWater_PasswordGate_2130": "2130",
    "DeepWater_PasswordHatch_1725": "1725",
}


def actor_label(actor):
    try:
        return actor.get_actor_label()
    except Exception:
        return actor.get_name()


def find_actor(actors, label):
    for actor in actors:
        if actor_label(actor) == label:
            return actor
    return None


def editor_property(obj, names):
    for name in names:
        try:
            return obj.get_editor_property(name)
        except Exception:
            continue
    return None


def primitive_component_by_name(actor, component_name):
    for component in actor.get_components_by_class(unreal.PrimitiveComponent):
        if component.get_name() == component_name:
            return component
    return None


def collision_summary(component):
    def read_value(callback, fallback="NA"):
        try:
            return str(callback())
        except Exception:
            return fallback

    enabled = read_value(component.get_collision_enabled)
    profile = read_value(component.get_collision_profile_name)
    pawn = read_value(lambda: component.get_collision_response_to_channel(unreal.CollisionChannel.ECC_PAWN))
    visibility = read_value(lambda: component.get_collision_response_to_channel(unreal.CollisionChannel.ECC_VISIBILITY))
    return f"{enabled}/{profile}/pawn={pawn}/visibility={visibility}"


def submit_password(actor, instigator, password):
    for function_name in ("submit_password", "SubmitPassword"):
        try:
            return getattr(actor, function_name)(instigator, password)
        except AttributeError:
            continue
    raise RuntimeError(f"{actor_label(actor)} does not expose SubmitPassword")


def has_no_collision(component):
    try:
        return component.get_collision_enabled() == unreal.CollisionEnabled.NO_COLLISION
    except Exception:
        return False


def verify_password_door(map_path, actors, instigator, label, expected_password):
    errors = []
    actor = find_actor(actors, label)
    if not actor:
        return [f"{map_path}: missing password door actor {label}"]

    requires_password = editor_property(actor, ("requires_password", "b_requires_password", "bRequiresPassword"))
    required_password = editor_property(actor, ("required_password", "RequiredPassword"))
    password_unlocked = editor_property(actor, ("password_unlocked", "b_password_unlocked", "bPasswordUnlocked"))
    start_locked = editor_property(actor, ("start_locked", "b_start_locked", "bStartLocked"))
    door_state = editor_property(actor, ("door_state", "DoorState"))
    door_mesh = primitive_component_by_name(actor, "DoorMesh")
    interaction_volume = primitive_component_by_name(actor, "InteractionVolume")

    print(
        "DAY1_PASSWORD_DOOR:DOOR:"
        f"map={map_path} label={label} requires={requires_password} "
        f"password={required_password} unlocked={password_unlocked} "
        f"start_locked={start_locked} state={door_state}"
    )

    if not requires_password:
        errors.append(f"{map_path}: {label} does not require a password")
    if str(required_password) != expected_password:
        errors.append(f"{map_path}: {label} password is {required_password}, expected {expected_password}")
    if password_unlocked:
        errors.append(f"{map_path}: {label} starts password_unlocked")
    if not start_locked:
        errors.append(f"{map_path}: {label} does not start locked")

    if door_mesh:
        print(
            "DAY1_PASSWORD_DOOR:DOOR_MESH:"
            f"map={map_path} label={label} collision={collision_summary(door_mesh)}"
        )
    else:
        errors.append(f"{map_path}: {label} missing DoorMesh")

    if interaction_volume:
        print(
            "DAY1_PASSWORD_DOOR:INTERACTION_VOLUME:"
            f"map={map_path} label={label} collision={collision_summary(interaction_volume)}"
        )
        try:
            if interaction_volume.get_collision_response_to_channel(unreal.CollisionChannel.ECC_VISIBILITY) != unreal.CollisionResponseType.ECR_BLOCK:
                errors.append(f"{map_path}: {label} interaction volume does not block visibility")
        except Exception as exc:
            errors.append(f"{map_path}: {label} interaction visibility check failed: {exc}")
    else:
        errors.append(f"{map_path}: {label} missing InteractionVolume")

    if door_mesh and instigator:
        accepted = submit_password(actor, instigator, expected_password)
        print(
            "DAY1_PASSWORD_DOOR:SUBMIT_PASSWORD:"
            f"map={map_path} label={label} accepted={accepted} "
            f"post_collision={collision_summary(door_mesh)}"
        )
        if not accepted:
            errors.append(f"{map_path}: {label} rejected the correct password")
        if not has_no_collision(door_mesh):
            errors.append(f"{map_path}: {label} still collides after correct password: {collision_summary(door_mesh)}")

    return errors


def main():
    errors = []
    for map_path in TARGET_MAPS:
        print(f"DAY1_PASSWORD_DOOR:LOAD_LEVEL:{map_path}")
        unreal.EditorLevelLibrary.load_level(map_path)
        actors = list(unreal.EditorLevelLibrary.get_all_level_actors())
        instigator = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.Actor, unreal.Vector(0.0, 0.0, 0.0))

        for label, expected_password in EXPECTED_PASSWORD_DOORS.items():
            errors.extend(verify_password_door(map_path, actors, instigator, label, expected_password))

    if errors:
        for error in errors:
            print(f"DAY1_PASSWORD_DOOR:ERROR:{error}")
        raise RuntimeError("Day 1 password door validation failed")

    print("DAY1_PASSWORD_DOOR:PASS")


main()
