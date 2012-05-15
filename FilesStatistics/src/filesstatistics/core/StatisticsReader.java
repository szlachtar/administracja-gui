/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package filesstatistics.core;

import java.io.IOException;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 *
 * @author Rafal
 */
public interface StatisticsReader {
    public static final String DATA_STRUCTURE_LIST_KEY = "dataStructures";
    public static final Set<String> OPERATIONS = new HashSet<String>();
    public static final String FILE_TYPE_KEY = "fileType";
    
    Map<String, Map<String, Object>> readStatistics() throws IOException;
}
