#include "ShamirSecretSharingHelper.h"


std::string ShamirSecretSharingHelper::reconstructSecret(int thresh, std::vector<std::string> shares)
{
    // ********** Recover secret
    int threshold = thresh;
    std::vector<std::string> strShares = shares;
    const unsigned int CHID_LENGTH = 4;
    std::string channel;

    try
    {
        std::string recovered;
        CryptoPP::SecretRecovery recovery(threshold, new CryptoPP::StringSink(recovered));

        CryptoPP::vector_member_ptrs<CryptoPP::StringSource> strSources(threshold);
        channel.resize(CHID_LENGTH);
        for (unsigned int i = 0; i < threshold; i++)
        {
            strSources[i].reset(new CryptoPP::StringSource(strShares[i], false));
            strSources[i]->Pump(CHID_LENGTH);
            strSources[i]->Get((CryptoPP::byte *)&channel[0], CHID_LENGTH);
            strSources[i]->Attach(new CryptoPP::ChannelSwitch(recovery, channel));
        }

        while (strSources[0]->Pump(256))
        {
            for (unsigned int i = 1; i < threshold; i++)
                strSources[i]->Pump(256);
        }

        for (unsigned int i = 0; i < threshold; i++)
            strSources[i]->PumpAll();

        return recovered;
    }
    catch (const CryptoPP::Exception &)
    {
        std::cout << "Es ist ein Fehler aufgetreten" << std::endl;
    }
}
