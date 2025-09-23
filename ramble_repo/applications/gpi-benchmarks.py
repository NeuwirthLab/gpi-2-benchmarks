
import os
import re
from enum import Enum

from ramble.appkit import *
from ramble.expander import Expander


class GpiBenchmarks(ExecutableApplication):
    """Define an application for gpi-benchmarks"""

    name = "gpi-benchmarks"

    maintainers("jwarczok", "nbarthel")

    tags(
        "gaspi-benchmark",
    )


    all_workloads = [
        "gbs_allreduce",
        "gbs_atomic_cas",
        "gbs_atomic_fadd",
        "gbs_barrier",
        "gbs_notification_ping_pong",
        "gbs_passive_bw",
        "gbs_passive_lat",
        "gbs_read_bw",
        "gbs_read_lat",
        "gbs_read_notify_bw",
        "gbs_read_notify_lat",
        "gbs_read_threads_bw",
        "gbs_write_bibw",
        "gbs_write_bw",
        "gbs_write_lat",
        "gbs_write_notify_ping_pong",
        "gbs_write_notify_bibw",
        "gbs_write_notify_bw",
        "gbs_write_notify_lat",
        "gbs_write_threads_bw"
    ]

    size_time_regex = r"(?P<msg_size>[0-9.]+)+\s+(?P<fom>[0-9.]+)"
    figure_of_merit_context(
        "msg_size",
        regex=size_time_regex,
        output_format="Message Size: {msg_size}",
    )

    log_str = Expander.expansion_str("log_file")

    for benchmark in all_workloads:
        executable(
            name=f"execute-{benchmark}",
            template=f"gaspi_run -m ~/machines /home/ohan/Arbeit/gpi-benchmarks/build/{benchmark} " + "{additional_args}",
            use_mpi=False,  
        )
        workload(benchmark, executable=f"execute-{benchmark}")

    workload_variable(
        "additional_args",
        default="",  
        description="Additional arguments for benchmark",
        workloads=all_workloads,
    )

    fom_types = Enum(
        "fom_types",
        [
            "atomic",
            "notify_ping_pong",
            "latency",
            "bandwidth",
            "allreduce",
            "barrier",
            "strided"
        ],
    )

    fom_regex_headers = {
        fom_types.atomic: re.compile(
            r"\s*#iterations\s+min_lat\s+max_lat\s+avg_lat\s+first_quartil_lat\s+median_lat\s+third_quartil_lat\s+var_lat\s+std_lat\s*$"),
        fom_types.notify_ping_pong: re.compile(
            r"\s*min_lat\s+max_lat\s+avg_lat\s+first_quartil_lat\s+median_lat\s+third_quartil_lat\s+var_lat\s+std_lat\s*$"),
        fom_types.latency: re.compile(
            r"\s*memory_mode\s+msg_size\s+min_lat\s+max_lat\s+avg_lat\s+first_quartil_lat\s+median_lat\s+third_quartil_lat\s+var_lat\s+std_lat\s*$"
        ),
        fom_types.bandwidth: re.compile(
            r"/s*memory_mode\s+msg_size\s+min_bw\s+max_bw\s+avg_bw\s+first_quartil_bw\s+median_bw\s+third_quartil_bw\s+var_bw\s+std_bw\s*$"
        ),
        fom_types.allreduce: re.compile(
            r"\s*memory_mode\s+#elements\s+#ranks\s+#iterations\s+min_lat\s+max_lat\s+avg_lat\s*$"
        ),
        fom_types.barrier: re.compile(r"\s*#ranks\s+#iterations\s+min_lat\s+max_lat\s+avg_lat\s*$"),
        fom_types.strided: re.compile(
            r"\s*#segments\s+#iterations\s+min_lat\s+max_lat\s+avg_lat\s+first_quartil_lat\s+median_lat\s+third_quartil_lat\s+var_lat\s+std_lat\s*$"
        )
    }


    group_mapping = {
        "#segments": {"name": "number of segments", "units": ""},
        "#iterations": {"name": "number of iterations", "units": ""},
        "#ranks": {"name": "number of ranks", "units": ""},
        "#elements": {"name": "number of elements", "units": ""},
        "memory_mode": {"name": "memory mode", "units": ""},
        "min_lat": {"name": "Min Latency", "units": "us"},
        "max_lat": {"name": "Max Latency", "units": "us"},
        "avg_lat": {"name": "Avg Latency", "units": "us"},
        "first_quartil_lat": {"name": "First Quartile Latency", "units": "us"},
        "median_latency": {"name": "Median Latency", "units": "us"},
        "third_quartil_lat": {"name": "Third Quartile Latency", "units": "us"},
        "var_lat": {"name": "Variance", "units": "(us)^2"},
        "std_lat": {"name": "StdDev Latency", "units": "us"},
        "msg_size": {"name": "Message Size", "units": "Bytes"},
        "min_bw": {"name": "Min Bandwidth", "units": "MB/s"},
        "max_bw": {"name": "Max Bandwidth", "units": "MB/s"},
        "avg_bw": {"name": "Avg Bandwidth", "units": "MB/s"},
        "first_quartil_bw": {"name": "First Quartile Bandwidth", "units": "MB/s"},
        "median_bw": {"name": "Median Bandwidth", "units": "MB/s"},
        "third_quartil_bw": {"name": "Third Quartile Bandwidth", "units": "MB/s"},
        "var_bw": {"name": "Variance", "units": "(MB/s)^2"},
        "std_bw": {"name": "StdDev Bandwidth", "units": "MB/s"},
    }


    def _add_foms(self, regex: str):
        """Add figures of merit based on group names from input regular expression

        Args:
            regex (str): An uncompiled regular expression to use for determining which groups
                         are going to be extracted.
        """
        if regex is None:
            return

        compiled = re.compile(regex)

        for grp_name, _ in compiled.groupindex.items():
            if grp_name in self.group_mapping:
                self.figure_of_merit(
                    self.group_mapping[grp_name]["name"],
                    fom_regex=regex,
                    group_name=grp_name,
                    units=self.group_mapping[grp_name]["units"],
                    contexts=["msg_size"],
                )

    def _prepare_analysis(self, workspace, app_inst=None):

        fom_type = None
        log_file = self.expander.expand_var_name("log_file")
        if os.path.isfile(log_file):
            with open(log_file) as f:
                for line in f.readlines():
                    for test_fom_type in self.fom_types:
                        if self.fom_regex_headers[test_fom_type].match(line):
                            fom_type = test_fom_type

                    if fom_type is not None:
                        break

        fom_regex = None
        if fom_type == self.fom_types.atomic:
            fom_regex = r"\s*(?P<iterations>[0-9]+)\s+(?P<min_lat>[0-9\.]+)\s+(?P<max_lat>[0-9\.]+)\s+(?P<avg_lat>[0-9\.]+)\s+(?P<first_quartil_lat>[0-9\.]+)\s+(?P<median_lat>[0-9\.]+)\s+(?P<third_quartil_lat>[0-9\.]+)\s+(?P<var_lat>[0-9\.]+)\s+(?P<std_lat>[0-9\.]+)\s*$"
            self._add_foms(fom_regex) #Warum ist das Hier, wenn es auch schon unten ist???
        elif fom_type == self.fom_types.notify_ping_pong:
            fom_regex = r"\s*(?P<min_lat>[0-9\.]+)\s+(?P<max_lat>[0-9\.]+)\s+(?P<avg_lat>[0-9\.]+)\s+(?P<first_quartil_lat>[0-9\.]+)\s+(?P<median_lat>[0-9\.]+)\s+(?P<third_quartil_lat>[0-9\.]+)\s+(?P<var_lat>[0-9\.]+)\s+(?P<std_lat>[0-9\.]+)\s*$"
        elif fom_type == self.fom_types.latency:
            fom_regex = r"\s*(?P<memory_mode>[a-z_]+)\s+(?P<msg_size>[0-9]+)\s+(?P<min_lat>[0-9\.]+)\s+(?P<max_lat>[0-9\.]+)\s+(?P<avg_lat>[0-9\.]+)\s+(?P<first_quartil_lat>[0-9\.]+)\s+(?P<median_lat>[0-9\.]+)\s+(?P<third_quartil_lat>[0-9\.]+)\s+(?P<var_lat>[0-9\.]+)\s+(?P<std_lat>[0-9\.]+)\s*$"
        elif fom_type == self.fom_types.bandwidth:
            fom_regex = r"\s*(?P<memory_mode>[a-z_]+)\s+(?P<msg_size>[0-9]+)\s+(?P<min_bw>[0-9\.]+)\s+(?P<max_bw>[0-9\.]+)\s+(?P<avg_bw>[0-9\.]+)\s+(?P<first_quartil_bw>[0-9\.]+)\s+(?P<median_bw>[0-9\.]+)\s+(?P<third_quartil_bw>[0-9\.]+)\s+(?P<var_bw>[0-9\.]+)\s+(?P<std_bw>[0-9\.]+)\s*$"
        elif fom_type == self.fom_types.allreduce:
            fom_regex = r"\s*(?P<memory_mode>[a-z_]+)\s+(?P<elements>[0-9]+)\s+(?P<ranks>[0-9]+)\s+(?P<iterations>[0-9]+)\s+(?P<min_lat>[0-9\.]+)\s+(?P<max_lat>[0-9\.]+)\s+(?P<avg_lat>[0-9\.]+)\s*$"
        elif fom_type == self.fom_types.barrier:
            fom_regex = r"\s*(?P<ranks>[0-9]+)\s+(?P<iterations>[0-9]+)\s+(?P<min_lat>[0-9\.]+)\s+(?P<max_lat>[0-9\.]+)\s+(?P<avg_lat>[0-9\.]+)\s*$"
        elif fom_type == self.fom_types.strided:
            fom_regex = r"\s*(?P<segments>[0-9]+)\s+(?P<iterations>[0-9]+)\s+(?P<min_lat>[0-9\.]+)\s+(?P<max_lat>[0-9\.]+)\s+(?P<avg_lat>[0-9\.]+)\s+(?P<first_quartil_lat>[0-9\.]+)\s+(?P<median_lat>[0-9\.]+)\s+(?P<third_quartil_lat>[0-9\.]+)\s+(?P<var_lat>[0-9\.]+)\s+(?P<std_lat>[0-9\.]+)\s*$"
        
        self._add_foms(fom_regex)
