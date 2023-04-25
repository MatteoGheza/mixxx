#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QDir>
#include <QString>
#include <atomic>
#include <memory>

#include "library/itunes/itunesdao.h"
#include "library/itunes/itunesimporter.h"
#include "library/itunes/itunesxmlimporter.h"
#include "test/mixxxtest.h"

class ITunesXMLImporterTest : public MixxxTest {
  protected:
    QDir getITunesTestDir() {
        return MixxxTest::getOrInitTestDir().filePath("itunes/");
    }

    std::unique_ptr<ITunesXMLImporter> makeImporter(
            const QString& xmlFileName, std::unique_ptr<ITunesDAO> dao) {
        QString xmlFilePath = getITunesTestDir().absoluteFilePath(xmlFileName);
        auto importer = std::make_unique<ITunesXMLImporter>(
                nullptr, xmlFilePath, cancelImport, std::move(dao));
        return importer;
    }

  private:
    std::atomic<bool> cancelImport;
};

class MockITunesDAO : public ITunesDAO {
  public:
    MOCK_METHOD(bool, importTrack, (const ITunesTrack&));
    MOCK_METHOD(bool, importPlaylist, (const ITunesPlaylist&));
    MOCK_METHOD(bool, importPlaylistRelation, (int, int));
    MOCK_METHOD(bool, importPlaylistTrack, (int, int, int));
    MOCK_METHOD(bool, applyPathMapping, (const ITunesPathMapping&));
};

using testing::_;
using testing::Return;

namespace {
std::unique_ptr<MockITunesDAO> makeMockDAO() {
    std::unique_ptr<MockITunesDAO> dao = std::make_unique<MockITunesDAO>();

    ON_CALL(*dao, importTrack(_)).WillByDefault(Return(true));
    ON_CALL(*dao, importPlaylist(_)).WillByDefault(Return(true));
    ON_CALL(*dao, importPlaylistRelation(_, _)).WillByDefault(Return(true));
    ON_CALL(*dao, applyPathMapping(_)).WillByDefault(Return(true));

    return dao;
}
} // anonymous namespace

TEST_F(ITunesXMLImporterTest, ParseMacOSMusicXML) {
    std::unique_ptr<MockITunesDAO> dao = makeMockDAO();

    int root = kRootITunesPlaylistId;
    EXPECT_CALL(*dao, importPlaylistRelation(root, 1172)); // Downloaded (built-in playlist)
    EXPECT_CALL(*dao, importPlaylistRelation(root, 1425)); // Folder A
    EXPECT_CALL(*dao, importPlaylistRelation(1425, 1498)); // - Folder B
    EXPECT_CALL(*dao, importPlaylistRelation(1498, 1431)); //   - Playlist A
    EXPECT_CALL(*dao, importPlaylistRelation(1498, 1436)); //   - Playlist B
    EXPECT_CALL(*dao, importPlaylistRelation(1425, 1494)); // - Playlist C
    EXPECT_CALL(*dao, importPlaylistRelation(root, 1440)); // Downloaded (smart playlist)
    EXPECT_CALL(*dao, importPlaylistRelation(root, 1449)); // Playlist D

    std::unique_ptr<ITunesXMLImporter> importer =
            makeImporter("macOS Music Library.xml", std::move(dao));
    importer->importLibrary();
}

TEST_F(ITunesXMLImporterTest, ParseITunesMusicXML) {
    std::unique_ptr<MockITunesDAO> dao = makeMockDAO();

    int root = kRootITunesPlaylistId;
    EXPECT_CALL(*dao, importPlaylistRelation(root, 98));  // Downloaded (built-in playlist)
    EXPECT_CALL(*dao, importPlaylistRelation(root, 125)); // Downloaded (built-in playlist)
    EXPECT_CALL(*dao, importPlaylistRelation(root, 134)); // Downloaded (built-in playlist)
    EXPECT_CALL(*dao, importPlaylistRelation(root, 140)); // Podcasts (built-in playlist)
    EXPECT_CALL(*dao, importPlaylistRelation(root, 147)); // Audiobooks (built-in playlist)
    EXPECT_CALL(*dao, importPlaylistRelation(root, 150)); // Genius (built-in playlist)
    EXPECT_CALL(*dao, importPlaylistRelation(root, 153)); // Folder A
    EXPECT_CALL(*dao, importPlaylistRelation(153, 159));  // - Folder B
    EXPECT_CALL(*dao, importPlaylistRelation(159, 165));  //   - Playlist A
    EXPECT_CALL(*dao, importPlaylistRelation(159, 170));  //   - Playlist B
    EXPECT_CALL(*dao, importPlaylistRelation(153, 174));  // - Playlist C
    EXPECT_CALL(*dao, importPlaylistRelation(root, 177)); // Downloaded (smart playlist)
    EXPECT_CALL(*dao, importPlaylistRelation(root, 186)); // Playlist D

    std::unique_ptr<ITunesXMLImporter> importer =
            makeImporter("iTunes Music Library.xml", std::move(dao));
    importer->importLibrary();
}
