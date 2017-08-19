
#include "TerranBot.h"

int main(int argc, char* argv[]) {
    Coordinator coordinator;
    coordinator.LoadSettings(argc, argv);

	coordinator.SetRealtime(true); //to run the game on regular speed

    TerranBot bot;
	coordinator.SetParticipants({
		//		CreateParticipant(Race::Protoss, new Agent()),
				CreateParticipant(Race::Terran, &bot),
				CreateComputer(Race::Zerg, Difficulty::Medium)
		
    });

    coordinator.LaunchStarcraft();
    coordinator.StartGame(sc2::kMapBelShirVestigeLE);

    while (coordinator.Update()) {

    }

    return 0;
}