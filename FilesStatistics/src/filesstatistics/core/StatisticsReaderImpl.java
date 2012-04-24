/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package filesstatistics.core;

import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Rafal
 */
public class StatisticsReaderImpl implements StatisticsReader {

    @Override
    public List<DataStructure> readStatistics() {
        List<DataStructure> files = new ArrayList<DataStructure>();
        
        DataStructure structure = new DataStructure("/dev/null", 54, 97);
        files.add(structure);
        structure = new DataStructure("/home/student/student.file", 999, 666);
        files.add(structure);
        structure = new DataStructure("/etc/passwd", 9999, 34);
        files.add(structure);
        
        return files;
    }
    
}
