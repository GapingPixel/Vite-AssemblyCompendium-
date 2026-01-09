FTransform USDGA_WallRun::GetPlayerCameraTransform() const {
if (!Character) {
return FTransform::Identity;
}

const UATPCCameraComponent* AtpCameraComponent = Character->FindComponentByClass<UATPCCameraComponent>();
if (!AtpCameraComponent) {
UE_LOG(LogTemp, Warning, TEXT("Character has no UATPCCameraComponent."));
return FTransform::Identity;
}

// Get the ATP camera's transform
const FVector AtpCameraLocation = AtpCameraComponent->GetCameraLocation();
const FRotator AtpCameraRotation = AtpCameraComponent->GetComponentRotation();

// Get the player's location
const FVector PlayerLocation = Character->GetActorLocation();

// Calculate the distance from ATP camera to player
const float DistanceFromAtpCamToPlayer = (AtpCameraLocation - PlayerLocation).Size();

// Get the direction from player to ATP camera
const FVector PlayerToAtpCam = (AtpCameraLocation - PlayerLocation).GetSafeNormal();

// Check if the ATP camera is already inside the wall
// We'll use a trace to determine this
FHitResult HitResult;
const FVector TraceStart(PlayerLocation.X, PlayerLocation.Y, CurrentWallLocation.Z);
const FVector TraceEnd(AtpCameraLocation.X, AtpCameraLocation.Y, CurrentWallLocation.Z);

// draw debug line
DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 15.f, 0, 3.f);

// Perform a trace to check if the ATP camera is inside the wall
const bool bIsAtpCameraInsideWall = !GetWorld()->LineTraceSingleByChannel(
HitResult, 
TraceStart, 
TraceEnd, 
ECC_WorldStatic, 
FCollisionQueryParams()
);

FVector NewCameraLocation;
FRotator NewCameraRotation;

if (bIsAtpCameraInsideWall) {
// If the ATP camera is already inside the wall, use its transform directly
NewCameraLocation = AtpCameraLocation;
NewCameraRotation = AtpCameraRotation;
} 
else {
// If the ATP camera is outside the wall, we need to find a position inside the wall
// that maintains the same distance from the player

// First, determine the wall normal based on the wall side
FVector WallNormal;
if (WallSide == EWallRunDirection::Right) {
WallNormal = FVector(0, 0, 1).Cross(PlayerToAtpCam).GetSafeNormal();
} 
else {
WallNormal = FVector(0, 0, -1).Cross(PlayerToAtpCam).GetSafeNormal();
}

// Calculate the angle between the player-to-camera vector and the wall normal
const float DotProduct = FVector::DotProduct(PlayerToAtpCam, WallNormal);
const float AngleToWall = FMath::Acos(DotProduct);

// If the angle is greater than 90 degrees, we need to rotate the camera
if (AngleToWall > FMath::DegreesToRadians(90.0f)) {
// Calculate the rotation needed to move the camera inside the wall
// We'll rotate around the axis perpendicular to both the wall normal and the player-to-camera vector
const FVector RotationAxis = FVector::CrossProduct(PlayerToAtpCam, WallNormal).GetSafeNormal();
const float RotationAngle = FMath::DegreesToRadians(90.0f) - AngleToWall;

// Create a rotation quaternion
const FQuat RotationQuat = FQuat(RotationAxis, RotationAngle);

// Apply the rotation to the player-to-camera vector
const FVector RotatedDirection = RotationQuat.RotateVector(PlayerToAtpCam);

// Calculate the new camera position
NewCameraLocation = PlayerLocation + (RotatedDirection * DistanceFromAtpCamToPlayer);

// Calculate the new camera rotation
// We want to maintain the ATP camera's pitch and roll, but adjust the yaw
const FRotator LookAtRotation = (PlayerLocation - NewCameraLocation).Rotation();
NewCameraRotation = FRotator(AtpCameraRotation.Pitch, LookAtRotation.Yaw, AtpCameraRotation.Roll);
} 
else {
// If the angle is less than 90 degrees, the camera is already inside the wall
// This shouldn't happen based on our earlier check, but just in case
NewCameraLocation = AtpCameraLocation;
NewCameraRotation = AtpCameraRotation;
}
}
UE_LOG(LogTemp, Verbose, TEXT("ATP Camera: %s, New Camera: %s, Inside Wall: %d"), 
*AtpCameraLocation.ToString(), *NewCameraLocation.ToString(), bIsAtpCameraInsideWall);

return FTransform(NewCameraRotation, NewCameraLocation);
}