/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package filesstatistics.core;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author Rafal
 */
public class StatisticsReaderImpl implements StatisticsReader {
    
    private Properties applicationProperties = null;
    
    public StatisticsReaderImpl() throws IOException {
        applicationProperties = PropertiesReader.readProperties();
    }

    @Override
    public Map<String, Map<String, Object>> readStatistics() throws IOException {
        
        FileReader statisticsFile = new FileReader((String)applicationProperties.get(PropertiesReader.STATISTICS_FILE_PATH_KEY));
        BufferedReader br = new BufferedReader(statisticsFile);
        
        Map<String, Map<String, Object>> statistics = new HashMap<String, Map<String,Object>>();
        
        String line = null;
        while((line = br.readLine()) != null) {
            DataStructure ds = parseLine(line);
            if(!statistics.containsKey(ds.getFileName())) {
                statistics.put(ds.getFileName(), new HashMap<String, Object>());
            }
            Map<String, Object> descriptionMap = statistics.get(ds.getFileName());
            if(!descriptionMap.containsKey(DATA_STRUCTURE_LIST_KEY)) {
                descriptionMap.put(DATA_STRUCTURE_LIST_KEY, new ArrayList<DataStructure>());
                descriptionMap.put(FILE_TYPE_KEY, ds.getFileType());
            }
            
            List<DataStructure> dsList = (List<DataStructure>)descriptionMap.get(DATA_STRUCTURE_LIST_KEY);
            dsList.add(ds);
            
            if(descriptionMap.containsKey(ds.getOperation())) {
                int count = (Integer)descriptionMap.get(ds.getOperation());
                count++;
                descriptionMap.put(ds.getOperation(), count);
            } else {
                OPERATIONS.add(ds.getOperation());
                descriptionMap.put(ds.getOperation(), 1);
            }
        }
        
        return statistics;
    }
    
    private DataStructure parseLine(String line) {
        String[] fields = line.split(" ");
        DataStructure ds = new DataStructure();
        ds.setFileName(fields[0]);
        ds.setFileType(fields[1]);
        ds.setOperation(fields[2]);
        ds.setDate(Long.parseLong(fields[3]));
        return ds;
    }
}
