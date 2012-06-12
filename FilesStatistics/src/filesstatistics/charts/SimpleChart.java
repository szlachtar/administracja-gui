/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package filesstatistics.charts;

import filesstatistics.core.DataStructure;
import filesstatistics.core.StatisticsReader;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.data.category.DefaultCategoryDataset;
import org.jfree.data.time.Day;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.data.xy.XYSeries;

/**
 *
 * @author Rafal
 */
public class SimpleChart {

    private StatisticsReader statisticsReader;
    private Map<String, Map<String, Object>> stats;
    private String file;
    private String operation;

    public SimpleChart(Map<String, Map<String, Object>> stats, String fileName, String operation) {
        this.stats = stats;
        this.file = fileName;
        this.operation = operation;
    }

    public JFreeChart createChart() {
        DateAxis dateAxis = new DateAxis("Date");
        DateFormat chartFormatter = new SimpleDateFormat("yyyy/MM/dd");
        dateAxis.setDateFormatOverride(chartFormatter);

        DefaultCategoryDataset xyDataset = new DefaultCategoryDataset();
        
        List<DataStructure> dataStructures = (List)stats.get(file).get("dataStructures");
        
        TreeMap<String, Integer> values = new TreeMap<String, Integer>();
        
        for(DataStructure ds : dataStructures) {
            if(!ds.getOperation().equals(operation)) {
                continue;
            }
            String date = chartFormatter.format(new Date(ds.getDate()));
            boolean containsKey = values.containsKey(date);
            Integer v = 0;
            if(containsKey) {
                v = values.get(date);
            }
            v++;
            values.put(date, v);
        }
        int count = 0;
        for(String date : values.keySet()) {
            count += values.get(date);
            xyDataset.addValue(count, file, date);
            
        }

        /*Integer c = (Integer)descriptionMap.get("M");
        if(c!=null){
            xyDataset.addValue(c, "M", file);
        }
        c = (Integer)descriptionMap.get("C");
        if(c!=null){
            xyDataset.addValue(c, "C", file);

        }c = (Integer) descriptionMap.get("A");
        if(c!=null){
            xyDataset.addValue(c, "A", file);
        }c = (Integer) descriptionMap.get("O");
        if(c!=null){
            xyDataset.addValue(c, "O", file);
        }*/

        JFreeChart chart = ChartFactory.createStackedBarChart(file + " - operation " + operation,
                "Dates",
                "Count",
                xyDataset,
                PlotOrientation.VERTICAL,
                false,
                true,
                false);

        return chart;
    }
}
